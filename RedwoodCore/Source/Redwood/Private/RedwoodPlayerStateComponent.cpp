// Copyright Incanta LLC. All rights reserved.

#include "RedwoodPlayerStateComponent.h"
#include "RedwoodClientGameSubsystem.h"
#include "RedwoodClientInterface.h"
#include "RedwoodGameModeComponent.h"
#include "RedwoodModule.h"
#include "RedwoodPlayerState.h"
#include "RedwoodServerGameSubsystem.h"
#include "RedwoodZoneSpawn.h"

#include "Engine/NetConnection.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/OnlineEngineInterface.h"

URedwoodPlayerStateComponent::URedwoodPlayerStateComponent(
  const FObjectInitializer &ObjectInitializer
) :
  Super(ObjectInitializer) {

  PrimaryComponentTick.bCanEverTick = true;

  OwnerPlayerState = Cast<APlayerState>(GetOwner());
  if (!OwnerPlayerState.IsValid()) {
    if (IsValid(GetOwner())) {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT(
          "RedwoodPlayerStateComponent requires a PlayerState owner, but attached to %s."
        ),
        *GetOwner()->GetName()
      );
    }

    return;
  }

  if (Cast<ARedwoodPlayerState>(GetOwner())) {
    UE_LOG(
      LogRedwood,
      Warning,
      TEXT(
        "ARedwoodPlayerState is deprecated and will be removed in 5.0.0. Migrate to using URedwoodPlayerStateComponent on any other APlayerState actor."
      ),
      *GetOwner()->GetName()
    );
  }

  OwnerPlayerState->PrimaryActorTick.bCanEverTick = true;
}

// Max wall-clock time the owning client keeps retrying TrySubmitJoinToken
// before giving up. Bounds the per-frame retry on PlayerStates that never
// resolve to a local controller (e.g. remote players' PlayerStates, which
// also tick this component). Comfortably exceeds the server's
// PendingAuth timeout, so a real owning client always gets its chance.
static constexpr float JoinTokenSubmitTimeoutSeconds = 30.0f;

void URedwoodPlayerStateComponent::BeginPlay() {
  Super::BeginPlay();

  // On the owning client only, hand the realm-frontend-issued bearer
  // token to the game server. The server defers sidecar auth
  // (URedwoodGameModeComponent::Login stashes a PendingAuth entry
  // keyed by the connection) until this RPC arrives. Sent via RPC
  // rather than as a URL option so the token never lands in LogNet URL
  // prints.
  //
  // Only NM_Client submits: on a dedicated/listen/standalone host the
  // local PlayerState has authority and the server already holds the
  // connection's auth state, so there's nothing to send.
  //
  // NOTE: we deliberately do NOT gate on ROLE_AutonomousProxy here. A
  // PlayerState is always ROLE_SimulatedProxy on clients — even the
  // owning client's own (see APlayerState ctor:
  // SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy)) — so that
  // check would never pass. We instead identify our PlayerState by its
  // owning controller being the local controller (in TrySubmitJoinToken).
  UWorld *World = GetWorld();
  if (World == nullptr || World->GetNetMode() != NM_Client) {
    return;
  }

  // The owning PlayerController may not be linked to this PlayerState yet
  // at BeginPlay, so arm a retry that TickComponent drives until the
  // token is sent (or we time out).
  bWantsToSubmitJoinToken = true;
  TrySubmitJoinToken();
}

void URedwoodPlayerStateComponent::TrySubmitJoinToken() {
  if (!bWantsToSubmitJoinToken || bJoinTokenSubmitted) {
    return;
  }

  APlayerState *PS = OwnerPlayerState.Get();
  if (PS == nullptr) {
    return;
  }

  // Is this our local player's PlayerState? On a client only the local
  // PlayerController(s) are present/linked — remote players' PCs aren't
  // replicated to us — so an owning controller that IsLocalController()
  // means this PlayerState belongs to a local player. If the owner isn't
  // linked yet, we'll be retried next tick.
  APlayerController *PC = Cast<APlayerController>(PS->GetOwningController());
  if (PC == nullptr || !PC->IsLocalController()) {
    return;
  }

  UWorld *World = GetWorld();
  UGameInstance *GameInstance = World ? World->GetGameInstance() : nullptr;
  if (GameInstance == nullptr) {
    return;
  }
  URedwoodClientGameSubsystem *Subsystem =
    GameInstance->GetSubsystem<URedwoodClientGameSubsystem>();
  if (Subsystem == nullptr) {
    return;
  }
  URedwoodClientInterface *ClientInterface = Subsystem->GetClientInterface();
  if (ClientInterface == nullptr) {
    return;
  }
  const FString &Token = ClientInterface->GetServerJoinToken();
  if (Token.IsEmpty()) {
    // Local play, PIE without backend, or this PlayerState arrived for
    // some non-Redwood reason — nothing to submit. Stop retrying.
    bWantsToSubmitJoinToken = false;
    return;
  }

  Server_SubmitJoinToken(Token);
  bJoinTokenSubmitted = true;
  bWantsToSubmitJoinToken = false;
}

void URedwoodPlayerStateComponent::Server_SubmitJoinToken_Implementation(
  const FString &Token
) {
  // Server side. The RPC routes to URedwoodGameModeComponent for the
  // actual sidecar verification — keeping that logic centralized so
  // the URL-Token legacy path and the RPC path share the same
  // verification implementation.
  AActor *Owner = GetOwner();
  if (Owner == nullptr) {
    return;
  }
  APlayerState *PS = Cast<APlayerState>(Owner);
  if (PS == nullptr) {
    return;
  }
  APlayerController *PC = Cast<APlayerController>(PS->GetOwningController());
  if (PC == nullptr) {
    UE_LOG(
      LogRedwood,
      Warning,
      TEXT(
        "Server_SubmitJoinToken from PlayerState with no owning player controller; "
        "dropping"
      )
    );
    return;
  }
  UNetConnection *NetConn = PC->GetNetConnection();
  if (NetConn == nullptr) {
    UE_LOG(
      LogRedwood,
      Warning,
      TEXT(
        "Server_SubmitJoinToken from a player controller with no net "
        "connection; dropping"
      )
    );
    return;
  }

  UWorld *World = GetWorld();
  AGameModeBase *GameMode = World ? World->GetAuthGameMode() : nullptr;
  if (GameMode == nullptr) {
    return;
  }
  URedwoodGameModeComponent *RedwoodGM =
    GameMode->FindComponentByClass<URedwoodGameModeComponent>();
  if (RedwoodGM == nullptr) {
    UE_LOG(
      LogRedwood,
      Warning,
      TEXT(
        "Server_SubmitJoinToken received but the GameMode has no "
        "URedwoodGameModeComponent; dropping"
      )
    );
    return;
  }

  RedwoodGM->ReceiveClientAuthToken(NetConn, Token);
}

bool URedwoodPlayerStateComponent::Server_SubmitJoinToken_Validate(
  const FString &Token
) {
  // Cheap shape check. The real verification happens against the
  // sidecar in URedwoodGameModeComponent::RunSidecarPlayerAuth; this
  // just rejects the obviously-bad cases before consuming a tick.
  return Token.Len() > 0 && Token.Len() <= 256;
}

void URedwoodPlayerStateComponent::TickComponent(
  float DeltaTime,
  enum ELevelTick TickType,
  FActorComponentTickFunction *ThisTickFunction
) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  // Owning-client retry for the join-token submission: the owning
  // PlayerController may not have been linked at BeginPlay. Keep trying
  // until we send it, or give up after the timeout (e.g. on remote
  // players' PlayerStates, which never resolve to a local controller).
  if (bWantsToSubmitJoinToken) {
    JoinTokenSubmitElapsed += DeltaTime;
    if (JoinTokenSubmitElapsed >= JoinTokenSubmitTimeoutSeconds) {
      bWantsToSubmitJoinToken = false;
    } else {
      TrySubmitJoinToken();
    }
  }

  if (bFollowPawn) {
    UWorld *World = GetWorld();

    if (
      IsValid(World) &&
      (
        World->GetNetMode() == ENetMode::NM_DedicatedServer ||
        World->GetNetMode() == ENetMode::NM_ListenServer ||
        World->GetNetMode() == ENetMode::NM_Standalone
      )
    ) {
      if (APlayerState *PlayerState = OwnerPlayerState.Get()) {
        APawn *Pawn = PlayerState->GetPawn();
        AActor *PlayerStateActor = Cast<AActor>(PlayerState);
        if (IsValid(Pawn)) {
          PlayerStateActor->SetActorLocation(Pawn->GetActorLocation());
          PlayerStateActor->SetActorRotation(Pawn->GetActorRotation());
        }
      }
    }
  }
}

void URedwoodPlayerStateComponent::SetClientReady_Implementation() {
  if (AActor *Owner = GetOwner()) {
    if (Owner->GetLocalRole() == ROLE_Authority) {
      bClientReady = true;
    }
  }
}

bool URedwoodPlayerStateComponent::SetClientReady_Validate() {
  return true;
}

void URedwoodPlayerStateComponent::SetServerReady() {
  if (AActor *Owner = GetOwner()) {
    if (Owner->GetLocalRole() == ROLE_Authority) {
      bServerReady = true;
    }
  }
}

void URedwoodPlayerStateComponent::InitTransferring() {
  bTransferring = true;

  // Notify the owning client. The Client RPC routes through the
  // PlayerState's owning controller's net connection, so only the
  // player being transferred receives it. On a standalone/listen host
  // the implementation runs locally and broadcasts directly.
  Client_OnTransferring();
}

void URedwoodPlayerStateComponent::Client_OnTransferring_Implementation() {
  OnTransferring.Broadcast();
}

void URedwoodPlayerStateComponent::SetRedwoodPlayer(
  FRedwoodPlayerData InRedwoodPlayer
) {
  RedwoodPlayer = InRedwoodPlayer;

  OnRedwoodPlayerUpdated.Broadcast();
}

void URedwoodPlayerStateComponent::SetRedwoodCharacter(
  FRedwoodCharacterBackend InRedwoodCharacter
) {
  RedwoodCharacter = InRedwoodCharacter;

  if (APlayerState *PlayerState = OwnerPlayerState.Get()) {
    FUniqueNetIdWrapper UniqueNetIdWrapper =
      UOnlineEngineInterface::Get()->CreateUniquePlayerIdWrapper(
        RedwoodCharacter.PlayerId + TEXT(":") + RedwoodCharacter.Id,
        FName(TEXT("RedwoodMMO"))
      );
    FUniqueNetIdRepl NetUniqueId(UniqueNetIdWrapper.GetUniqueNetId());

    PlayerState->SetUniqueId(NetUniqueId);
  }

  OnRedwoodCharacterUpdated.Broadcast();
}

bool URedwoodPlayerStateComponent::GetSpawnData(
  FTransform &Transform, FRotator &ControlRotation
) {
  if (RedwoodCharacter.RedwoodData != nullptr && RedwoodCharacter.RedwoodData->IsValid()) {
    URedwoodServerGameSubsystem *RedwoodServerGameSubsystem =
      GetWorld()->GetGameInstance()->GetSubsystem<URedwoodServerGameSubsystem>(
      );

    if (!RedwoodServerGameSubsystem) {
      return false;
    }

    USIOJsonObject *LastLocation;
    if (RedwoodCharacter.RedwoodData->TryGetObjectField(
          TEXT("lastLocation"), LastLocation
        )) {
      FString LastZoneName = LastLocation->GetStringField(TEXT("zoneName"));

      if (LastZoneName != RedwoodServerGameSubsystem->ZoneName) {
        return false;
      }

      FString LastSpawnName;
      if (LastLocation->TryGetStringField(TEXT("spawnName"), LastSpawnName)) {
        TArray<AActor *> ZoneSpawns;
        UGameplayStatics::GetAllActorsOfClass(
          GetWorld(), ARedwoodZoneSpawn::StaticClass(), ZoneSpawns
        );

        TArray<ARedwoodZoneSpawn *> RedwoodZoneSpawns;
        for (AActor *ZoneSpawn : ZoneSpawns) {
          ARedwoodZoneSpawn *RedwoodZoneSpawn =
            Cast<ARedwoodZoneSpawn>(ZoneSpawn);
          if (IsValid(RedwoodZoneSpawn)) {
            if (RedwoodZoneSpawn->ZoneName == RedwoodServerGameSubsystem->ZoneName && RedwoodZoneSpawn->SpawnName == LastSpawnName) {
              FTransform SpawnTransform = RedwoodZoneSpawn->GetSpawnTransform();
              Transform.SetLocation(SpawnTransform.GetLocation());
              Transform.SetRotation(SpawnTransform.GetRotation());
              Transform.SetScale3D(SpawnTransform.GetScale3D());

              FRotator SpawnRotation = SpawnTransform.GetRotation().Rotator();
              ControlRotation.SetComponentForAxis(
                EAxis::X, SpawnRotation.GetComponentForAxis(EAxis::X)
              );
              ControlRotation.SetComponentForAxis(
                EAxis::Y, SpawnRotation.GetComponentForAxis(EAxis::Y)
              );
              ControlRotation.SetComponentForAxis(
                EAxis::Z, SpawnRotation.GetComponentForAxis(EAxis::Z)
              );
              return true;
            }
          }
        }

        FString NotificationText = FString::Printf(
          TEXT(
            "Player had a lastLocation with spawnName %s, but no matching ARedwoodZoneSpawn could be found."
          ),
          *LastSpawnName
        );

        FRedwoodModule::ShowNotification(NotificationText);

        return false;
      }

      USIOJsonObject *LastTransform;

      if (LastLocation->TryGetObjectField(TEXT("transform"), LastTransform)) {
        USIOJsonObject *Location;
        if (!LastTransform->TryGetObjectField(TEXT("location"), Location)) {
          UE_LOG(
            LogRedwood,
            Error,
            TEXT("Invalid lastTransform (no location object field)")
          );

          FString NotificationText = FString::Printf(TEXT(
            "Player had a lastLocation with transform but with no location object field"
          ));

          FRedwoodModule::ShowNotification(NotificationText);

          return false;
        }

        USIOJsonObject *Rotation;
        if (!LastTransform->TryGetObjectField(TEXT("rotation"), Rotation)) {
          UE_LOG(
            LogRedwood,
            Error,
            TEXT("Invalid lastTransform (no rotation object field)")
          );

          FString NotificationText = FString::Printf(TEXT(
            "Player had a lastLocation with transform but with no rotation object field"
          ));

          FRedwoodModule::ShowNotification(NotificationText);

          return false;
        }

        float LocX = Location->GetNumberField(TEXT("x"));
        float LocY = Location->GetNumberField(TEXT("y"));
        float LocZ = Location->GetNumberField(TEXT("z"));

        float RotX = Rotation->GetNumberField(TEXT("x"));
        float RotY = Rotation->GetNumberField(TEXT("y"));
        float RotZ = Rotation->GetNumberField(TEXT("z"));

        Transform =
          FTransform(FRotator(RotY, RotZ, RotX), FVector(LocX, LocY, LocZ));

        USIOJsonObject *ControlRotationObject;
        if (!LastTransform->TryGetObjectField(
              TEXT("controlRotation"), ControlRotationObject
            )) {
          UE_LOG(
            LogRedwood,
            Error,
            TEXT("Invalid lastTransform (no controlRotation object field)")
          );

          FString NotificationText = FString::Printf(TEXT(
            "Player had a lastLocation with transform but with no controlRotation object field"
          ));

          FRedwoodModule::ShowNotification(NotificationText);

          return false;
        }

        float Roll = ControlRotationObject->GetNumberField(TEXT("x"));
        float Pitch = ControlRotationObject->GetNumberField(TEXT("y"));
        float Yaw = ControlRotationObject->GetNumberField(TEXT("z"));

        ControlRotation = FRotator(Pitch, Yaw, Roll);

        return true;
      }
    }
  }

  return false;
}

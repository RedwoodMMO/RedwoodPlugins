// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodGameModeComponent.h"
#include "RedwoodCommonGameSubsystem.h"
#include "RedwoodGameplayTags.h"
#include "RedwoodModule.h"
#include "RedwoodPlayerState.h"
#include "RedwoodServerGameSubsystem.h"
#include "RedwoodZoneSpawn.h"

#if WITH_EDITOR
  #include "RedwoodEditorSettings.h"
#endif

#include "Dom/JsonObject.h"
#include "Engine/NetConnection.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Guid.h"
#include "Net/OnlineEngineInterface.h"

#include "SocketIOClient.h"

URedwoodGameModeComponent::URedwoodGameModeComponent(
  const FObjectInitializer &ObjectInitializer
) :
  Super(ObjectInitializer) {
  PrimaryComponentTick.bStartWithTickEnabled = true;
  PrimaryComponentTick.bCanEverTick = true;
}

void URedwoodGameModeComponent::BeginPlay() {
  Super::BeginPlay();

  UE_LOG(
    LogRedwood,
    Log,
    TEXT(
      "RedwoodGameModeComponent BeginPlay, will attempt to run PostBeginPlay every %f seconds until initialized"
    ),
    PostBeginPlayDelay
  );

  FTimerManager &TimerManager = GetWorld()->GetTimerManager();
  TimerManager.SetTimer(
    PostBeginPlayTimerHandle,
    this,
    &URedwoodGameModeComponent::PostBeginPlay,
    PostBeginPlayDelay,
    true
  );

  // Periodically kick connections that authenticated via the
  // RedwoodAuth URL path but never invoked Server_SubmitJoinToken.
  TimerManager.SetTimer(
    PendingAuthPruneTimerHandle,
    this,
    &URedwoodGameModeComponent::PruneStalePendingAuth,
    /* InRate */ 5.0f,
    /* bLoop */ true
  );
}

void URedwoodGameModeComponent::PostBeginPlay() {
  ServerSubsystem =
    GetWorld()->GetGameInstance()->GetSubsystem<URedwoodServerGameSubsystem>();

  if (ServerSubsystem) {
    UE_LOG(
      LogRedwood,
      Log,
      TEXT(
        "URedwoodGameModeComponent::PostBeginPlay: Valid RedwoodServerGameSubsystem found, finishing initialization"
      )
    );

    FTimerManager &TimerManager = GetWorld()->GetTimerManager();
    TimerManager.ClearTimer(PostBeginPlayTimerHandle);

    ServerSubsystem->InitialDataLoad(FRedwoodDelegate::CreateLambda([this]() {
      UE_LOG(
        LogRedwood,
        Log,
        TEXT(
          "URedwoodGameModeComponent::PostBeginPlay: Initial data load complete"
        )
      );

      bPostBeganPlay = true;

      // create a looping timer to flush persistent data
      if (DatabasePersistenceInterval > 0) {
        FTimerManager &TimerManager = GetWorld()->GetTimerManager();
        TimerManager.SetTimer(
          FlushPersistentDataTimerHandle,
          this,
          &URedwoodGameModeComponent::FlushPersistence,
          DatabasePersistenceInterval,
          true
        );
      }
    }));
  } else {
    UE_LOG(
      LogRedwood,
      Warning,
      TEXT(
        "URedwoodGameModeComponent::PostBeginPlay: Invalid RedwoodServerGameSubsystem (likely during world initialization); will retry shortly"
      )
    );
  }
}

void URedwoodGameModeComponent::TickComponent(
  float DeltaTime,
  enum ELevelTick TickType,
  FActorComponentTickFunction *ThisTickFunction
) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (bPostBeganPlay && !GetWorld()->bIsTearingDown) {
    if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
      URedwoodServerGameSubsystem *RedwoodServerGameSubsystem =
        GetWorld()
          ->GetGameInstance()
          ->GetSubsystem<URedwoodServerGameSubsystem>();

      RedwoodServerGameSubsystem->FlushSync();
    }
  }
}

void URedwoodGameModeComponent::FlushPersistence() {
  ServerSubsystem->FlushPersistence();
}

void URedwoodGameModeComponent::InitGame(
  const FString &MapName, const FString &Options, FString &ErrorMessage
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    Sidecar = ISocketIOClientModule::Get().NewValidNativePointer();

    if (!ServerSubsystem) {
      ServerSubsystem = GetWorld()
                          ->GetGameInstance()
                          ->GetSubsystem<URedwoodServerGameSubsystem>();
    }

    if (ServerSubsystem) {
      Sidecar->Connect(ServerSubsystem->SidecarUri);
    } else {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT("Invalid RedwoodServerGameSubsystem; cannot connect to sidecar")
      );
    }
  }

  FGameModeEvents::GameModeLogoutEvent.AddUObject(
    this, &URedwoodGameModeComponent::OnGameModeLogout
  );
}

void URedwoodGameModeComponent::OnGameModeLogout(
  AGameModeBase *GameMode, AController *Controller
) {
  APlayerController *PlayerController = Cast<APlayerController>(Controller);
  if (PlayerController == nullptr) {
    return;
  }

  URedwoodPlayerStateComponent *PlayerStateComponent =
    IsValid(PlayerController->PlayerState)
    ? PlayerController->PlayerState
        ->FindComponentByClass<URedwoodPlayerStateComponent>()
    : nullptr;
  if (IsValid(PlayerStateComponent)) {
    if (ServerSubsystem) {
      TArray<APlayerState *> PlayerFlushArray;
      PlayerFlushArray.Add(PlayerController->PlayerState);
      ServerSubsystem->FlushPlayerCharacterData(PlayerFlushArray, true);
    }

    if (URedwoodCommonGameSubsystem::ShouldUseBackend(GameMode->GetWorld())) {
      if (Sidecar.IsValid() && Sidecar->bIsConnected) {
        TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
        JsonObject->SetStringField(
          TEXT("playerId"), PlayerStateComponent->RedwoodCharacter.PlayerId
        );
        JsonObject->SetStringField(
          TEXT("characterId"), PlayerStateComponent->RedwoodCharacter.Id
        );
        Sidecar->Emit(
          TEXT("realm:servers:player-left:game-server-to-sidecar"), JsonObject
        );
      }
    }

    if (UGameplayMessageSubsystem::HasInstance(this)) {
      // When we stop PIE, it's possible for the subsystem to be destroyed
      // before we get this event, so we need to check if it's valid
      UGameplayMessageSubsystem &MessageSubsystem =
        UGameplayMessageSubsystem::Get(this);
      MessageSubsystem.BroadcastMessage(
        TAG_Redwood_Player_Left, FRedwoodPlayerLeft{PlayerController}
      );
    }
  }
}

APlayerController *URedwoodGameModeComponent::Login(
  UPlayer *NewPlayer,
  ENetRole InRemoteRole,
  const FString &Portal,
  const FString &Options,
  const FUniqueNetIdRepl &UniqueId,
  FString &ErrorMessage,
  std::function<APlayerController
                  *(UPlayer *,
                    ENetRole,
                    const FString &,
                    const FString &,
                    const FUniqueNetIdRepl &,
                    FString &)> SuperDelegate
) {
  APlayerController *PlayerController = SuperDelegate(
    NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage
  );

  if (!ErrorMessage.IsEmpty() || PlayerController == nullptr) {
    return PlayerController;
  }

  UWorld *World = GetWorld();
  AGameModeBase *GameMode = World->GetAuthGameMode();

  if (URedwoodCommonGameSubsystem::ShouldUseBackend(World)) {
    FString PlayerId;
    FString CharacterId;
    FString Token;

#if WITH_EDITOR
    if (World->WorldType == EWorldType::PIE) {
      // Redwood has an option that is only available when the backend
      // the "dev-debug" game server provider is being used.
      int32 PlayerIndex = GameMode->GameState->PlayerArray.Num() - 1;
      PlayerId = FString::Printf(TEXT("development_%d"), PlayerIndex);
      CharacterId = FString::Printf(TEXT("development_%d"), PlayerIndex);
      Token = TEXT("development");
    }
#endif

    if (UGameplayStatics::HasOption(Options, TEXT("RedwoodAuth"))) {
      PlayerId = UGameplayStatics::ParseOption(Options, TEXT("PlayerId"));
      CharacterId = UGameplayStatics::ParseOption(Options, TEXT("CharacterId"));
      // Token is no longer carried in the URL. New clients send
      // it via the `URedwoodPlayerStateComponent::Server_SubmitJoinToken`
      // Server RPC after the connection is established (see
      // `ReceiveClientAuthToken`). The `Token=` URL option is still
      // read here as a legacy fallback so a server can be upgraded
      // ahead of clients; remove this fallback once all clients have
      // shipped with the Server-RPC path.
      Token = UGameplayStatics::ParseOption(Options, TEXT("Token"));
    }

    if (PlayerId.IsEmpty() || CharacterId.IsEmpty()) {
      ErrorMessage = TEXT(
        "Invalid authentication request: missing RedwoodAuth/PlayerId/CharacterId"
      );
      return PlayerController;
    }

    if (!Token.IsEmpty()) {
      // Legacy URL-token path: verify immediately.
      RunSidecarPlayerAuth(PlayerController, PlayerId, CharacterId, Token);
    } else {
      // Deferred-auth path: stash a pending entry keyed by this
      // connection and wait for the client to invoke
      // `Server_SubmitJoinToken`. PruneStalePendingAuth will kick
      // connections that never send one.
      UNetConnection *Connection =
        Cast<UNetConnection>(PlayerController->Player);
      if (Connection == nullptr) {
        ErrorMessage = TEXT(
          "Cannot defer auth: PlayerController has no UNetConnection"
        );
        return PlayerController;
      }

      FPendingAuth Pending;
      Pending.PlayerController = PlayerController;
      Pending.PlayerId = PlayerId;
      Pending.CharacterId = CharacterId;
      Pending.CreatedAtSeconds = FPlatformTime::Seconds();
      PendingAuthByConnection.Add(Connection, MoveTemp(Pending));

      UE_LOG(
        LogRedwood,
        Log,
        TEXT(
          "Player %s (character %s) connected; awaiting auth token via Server_SubmitJoinToken"
        ),
        *PlayerId,
        *CharacterId
      );
    }
  } else {
    // we're likely PIE so just load the character from disk based on num players
    uint8 PlayerIndex = 0;
    if (IsValid(GameMode->GameState)) {
      PlayerIndex = GameMode->GameState->PlayerArray.Num() - 1;
    } else {
      FString ErrorGameState = TEXT(
        "GameState is not valid yet, defaulting to player index 0 for character loading"
      );

      UE_LOG(LogRedwood, Error, TEXT("%s"), *ErrorGameState);

      FRedwoodModule::ShowNotification(ErrorGameState);
    }

    TArray<FRedwoodCharacterBackend> Characters =
      URedwoodCommonGameSubsystem::LoadAllCharactersFromDisk();

    if (PlayerIndex < Characters.Num()) {
      URedwoodPlayerStateComponent *PlayerStateComponent =
        PlayerController->PlayerState
          ->FindComponentByClass<URedwoodPlayerStateComponent>();
      if (IsValid(PlayerStateComponent)) {
        FRedwoodCharacterBackend &Character = Characters[PlayerIndex];
        PlayerStateComponent->SetRedwoodCharacter(Character);

        PlayerStateComponent->SetServerReady();

        // NOTE: we do not call HandleStartingNewPlayer here because we
        // don't need to. This all runs synchronously and PostLogin will
        // call HandleStartingNewPlayer for us. We only call HandleStartingNewPlayer
        // above because of the asynchronous nature of the backend call
      } else {
        UE_LOG(
          LogRedwood,
          Log,
          TEXT("Can't load character data as we're not using RedwoodPlayerState"
          )
        );

        FRedwoodModule::ShowNotification(TEXT(
          "Can't load character data as we're not using RedwoodPlayerState"
        ));
      }
    } else {
      ErrorMessage = FString::Printf(
        TEXT("No character found for this player index %d"), PlayerIndex
      );
      FString SubText =
        TEXT("Did you create one in Standalone with bUseBackendInPIE = false?");

      FRedwoodModule::ShowNotification(
        ErrorMessage, 10.0f, true, true, SubText
      );
    }
  }

  return PlayerController;
}

void URedwoodGameModeComponent::PostLogin(APlayerController *NewPlayer) {
  URedwoodPlayerStateComponent *PlayerStateComponent =
    NewPlayer->PlayerState->FindComponentByClass<URedwoodPlayerStateComponent>(
    );
  if (IsValid(PlayerStateComponent)) {
    PlayerStateComponent->bRanPostLogin = true;
  }
}

TArray<FString> URedwoodGameModeComponent::GetExpectedCharacterIds() const {
  TArray<FString> ExpectedCharacterIds;

  TArray<FString> Options = GetWorld()->URL.Op;
  for (const FString &Option : Options) {
    if (Option.StartsWith(TEXT("redwoodExpectedCharacterIds="))) {
      FString AllIds = Option.RightChop(28);

      for (int32 IdStart = 0, IdEnd = 0; IdEnd != INDEX_NONE;
           IdStart = IdEnd + 1) {
        IdEnd = AllIds.Find(
          TEXT(","), ESearchCase::IgnoreCase, ESearchDir::FromStart, IdStart
        );
        if (IdEnd == INDEX_NONE) {
          ExpectedCharacterIds.Add(AllIds.RightChop(IdStart));
          break;
        }
        ExpectedCharacterIds.Add(AllIds.Mid(IdStart, IdEnd - IdStart));
      }

      break;
    }
  }

  return ExpectedCharacterIds;
}

bool URedwoodGameModeComponent::PlayerCanRestart_Implementation(
  APlayerController *Player,
  std::function<bool(APlayerController *)> SuperDelegate
) {
  URedwoodPlayerStateComponent *PlayerStateComponent =
    Player->PlayerState->FindComponentByClass<URedwoodPlayerStateComponent>();
  if (IsValid(PlayerStateComponent)) {
    if (!PlayerStateComponent->bServerReady) {
      return false;
    }
  }

  return SuperDelegate(Player);
}

void URedwoodGameModeComponent::FinishRestartPlayer(
  AController *NewPlayer,
  const FRotator &StartRotation,
  std::function<void(AController *)> FailedToRestartPlayerDelegate
) {
  NewPlayer->Possess(NewPlayer->GetPawn());

  // If the Pawn is destroyed as part of possession we have to abort
  if (!IsValid(NewPlayer->GetPawn())) {
    FailedToRestartPlayerDelegate(NewPlayer);
  } else {
    URedwoodPlayerStateComponent *PlayerStateComponent =
      NewPlayer->PlayerState
        ->FindComponentByClass<URedwoodPlayerStateComponent>();

    FRotator NewControlRotation = NewPlayer->GetPawn()->GetActorRotation();

    if (IsValid(PlayerStateComponent)) {
      FTransform OutTransform;
      FRotator OutControlRotation;
      if (PlayerStateComponent->GetSpawnData(
            OutTransform, OutControlRotation
          )) {
        NewControlRotation = OutControlRotation;
      }
    }

    NewPlayer->ClientSetRotation(NewControlRotation, true);
    NewPlayer->SetControlRotation(NewControlRotation);

    AGameModeBase *GameMode = GetWorld()->GetAuthGameMode();
    GameMode->SetPlayerDefaults(NewPlayer->GetPawn());
    GameMode->K2_OnRestartPlayer(NewPlayer);
  }
}

FTransform URedwoodGameModeComponent::PickPawnSpawnTransform(
  AController *NewPlayer, const FTransform &SpawnTransform
) {
  URedwoodServerGameSubsystem *RedwoodServerGameSubsystem =
    NewPlayer->GetWorld()
      ->GetGameInstance()
      ->GetSubsystem<URedwoodServerGameSubsystem>();

  // get all actors of the ARedwoodZoneSpawn class
  TArray<AActor *> ZoneSpawns;
  UGameplayStatics::GetAllActorsOfClass(
    NewPlayer->GetWorld(), ARedwoodZoneSpawn::StaticClass(), ZoneSpawns
  );

  FString ZoneName = RedwoodServerGameSubsystem->ZoneName;

#if WITH_EDITOR
  const URedwoodEditorSettings *EditorSettings =
    GetDefault<URedwoodEditorSettings>();

  if (
    NewPlayer->GetWorld()->WorldType == EWorldType::PIE &&
    EditorSettings->bUseBackendInPIE == false &&
    ZoneName.IsEmpty() &&
    !EditorSettings->FallbackZoneName.IsEmpty()
  ) {
    ZoneName = EditorSettings->FallbackZoneName;
  }
#endif

  TArray<ARedwoodZoneSpawn *> RedwoodZoneSpawns;
  for (AActor *ZoneSpawn : ZoneSpawns) {
    ARedwoodZoneSpawn *RedwoodZoneSpawn = Cast<ARedwoodZoneSpawn>(ZoneSpawn);
    if (IsValid(RedwoodZoneSpawn)) {
      if (RedwoodZoneSpawn->ZoneName == ZoneName) {
        RedwoodZoneSpawns.Add(RedwoodZoneSpawn);
      }
    }
  }

  URedwoodPlayerStateComponent *PlayerStateComponent =
    NewPlayer->PlayerState->FindComponentByClass<URedwoodPlayerStateComponent>(
    );

  if (IsValid(PlayerStateComponent)) {
    FTransform OutTransform;
    FRotator OutControlRotation;
    if (PlayerStateComponent->GetSpawnData(OutTransform, OutControlRotation)) {
      return OutTransform;
    }

    UE_LOG(
      LogRedwood,
      Log,
      TEXT("No valid last transform found, using default zone spawn")
    );

    for (ARedwoodZoneSpawn *ZoneSpawn : RedwoodZoneSpawns) {
      if (ZoneSpawn->SpawnName == TEXT("default")) {
        return ZoneSpawn->GetSpawnTransform();
      }
    }

    if (RedwoodZoneSpawns.Num() > 0) {
      FRedwoodModule::ShowNotification(TEXT(
        "Could not find ARedwoodZoneSpawn with SpawnName 'default', using first available spawn"
      ));

      return RedwoodZoneSpawns[0]->GetSpawnTransform();
    } else {
      bool bShowNotification = true;

#if WITH_EDITOR
      bShowNotification = EditorSettings->bUseBackendInPIE ||
        !EditorSettings->FallbackZoneName.IsEmpty();
#endif

      FString NotificationText = FString::Printf(
        TEXT(
          "Could not find a valid spawn location for the player; using default transform (Loc %f, %f, %f)."
        ),
        SpawnTransform.GetLocation().X,
        SpawnTransform.GetLocation().Y,
        SpawnTransform.GetLocation().Z
      );

      if (bShowNotification) {
        FRedwoodModule::ShowNotification(NotificationText);
      }
    }
  } else {
    FString NotificationText = FString::Printf(
      TEXT(
        "Trying to spawn player without valid URedwoodPlayerStateComponent; using default transform (Loc %f, %f, %f)."
      ),
      SpawnTransform.GetLocation().X,
      SpawnTransform.GetLocation().Y,
      SpawnTransform.GetLocation().Z
    );
  }

  UE_LOG(
    LogRedwood,
    Error,
    TEXT(
      "Could not find a lastLocation for the character and there's no valid ARedwoodZoneSpawn found for this zone (%s). Using default transform."
    ),
    *RedwoodServerGameSubsystem->ZoneName
  );

  return SpawnTransform;
}

// ---------------------------------------------------------------------------
// Deferred player auth via URedwoodPlayerStateComponent::
//      Server_SubmitJoinToken Server RPC
// ---------------------------------------------------------------------------

namespace {
// Max wall-clock time a connection can sit in PendingAuthByConnection
// without the client invoking Server_SubmitJoinToken before we kick.
// Sized generously since the token travels over the same socket as
// the initial connect and arrives within a frame in practice.
constexpr double PendingAuthTimeoutSeconds = 30.0;
// How often the prune timer fires.
constexpr float PendingAuthPruneIntervalSeconds = 5.0f;
} // namespace

void URedwoodGameModeComponent::RunSidecarPlayerAuth(
  APlayerController *PlayerController,
  const FString &PlayerId,
  const FString &CharacterId,
  const FString &Token
) {
  if (!IsValid(PlayerController)) {
    return;
  }

  UWorld *World = GetWorld();
  AGameModeBase *GameMode = World ? World->GetAuthGameMode() : nullptr;
  if (GameMode == nullptr) {
    return;
  }

  if (!Sidecar.IsValid() || !Sidecar->bIsConnected) {
    UE_LOG(
      LogRedwood,
      Error,
      TEXT("Sidecar is not connected; kicking %s"),
      *PlayerId
    );
    GameMode->GameSession->KickPlayer(
      PlayerController, FText::FromString(TEXT("Sidecar is not connected"))
    );
    return;
  }

  TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
  JsonObject->SetStringField(TEXT("playerId"), PlayerId);
  JsonObject->SetStringField(TEXT("characterId"), CharacterId);
  JsonObject->SetStringField(TEXT("token"), Token);

  Sidecar->Emit(
    TEXT("realm:servers:player-auth:game-server-to-sidecar"),
    JsonObject,
    [this, PlayerId, PlayerController, GameMode](auto Response) {
      if (!IsValid(PlayerController)) {
        return;
      }
      TSharedPtr<FJsonObject> MessageStruct = Response[0]->AsObject();
      FString Error = MessageStruct->GetStringField(TEXT("error"));

      if (Error.IsEmpty()) {
        TSharedPtr<FJsonObject> Character =
          MessageStruct->GetObjectField(TEXT("character"));
        FString CharacterId = Character->GetStringField(TEXT("id"));
        FString CharacterName = Character->GetStringField(TEXT("name"));

        TSharedPtr<FJsonObject> Player =
          MessageStruct->GetObjectField(TEXT("player"));
        FString TempPlayerId = Player->GetStringField(TEXT("id"));

        URedwoodPlayerStateComponent *PlayerStateComponent =
          PlayerController->PlayerState
            ->FindComponentByClass<URedwoodPlayerStateComponent>();
        if (IsValid(PlayerStateComponent)) {
          UE_LOG(
            LogRedwood,
            Log,
            TEXT("Player joined as character %s"),
            *CharacterId
          );

          PlayerStateComponent->SetRedwoodPlayer(
            URedwoodCommonGameSubsystem::ParsePlayerData(Player)
          );
          PlayerStateComponent->SetRedwoodCharacter(
            URedwoodCommonGameSubsystem::ParseCharacter(Character)
          );
          PlayerStateComponent->SetServerReady();

          if (PlayerStateComponent->bRanPostLogin) {
            GameMode->HandleStartingNewPlayer(PlayerController);
          }
        } else {
          UE_LOG(
            LogRedwood,
            Log,
            TEXT(
              "Player joined as character %s (player %s), but we're not using RedwoodPlayerState"
            ),
            *CharacterId,
            *TempPlayerId
          );
        }
      } else {
        UE_LOG(
          LogRedwood,
          Error,
          TEXT("Player failed to authenticate, kicking them now: %s"),
          *Error
        );
        if (IsValid(GameMode) && IsValid(GameMode->GameSession)) {
          GameMode->GameSession->KickPlayer(
            PlayerController, FText::FromString(Error)
          );
        } else {
          UE_LOG(
            LogRedwood,
            Error,
            TEXT(
              "Failed to kick player after authentication failure because GameMode or GameSession was invalid"
            )
          );
        }
      }
    }
  );
}

void URedwoodGameModeComponent::ReceiveClientAuthToken(
  UNetConnection *Connection, const FString &Token
) {
  if (Connection == nullptr) {
    return;
  }

  FPendingAuth Pending;
  if (!PendingAuthByConnection.RemoveAndCopyValue(Connection, Pending)) {
    UE_LOG(
      LogRedwood,
      Warning,
      TEXT(
        "Received auth-token Server RPC from a connection with no pending "
        "entry; kicking"
      )
    );
    if (Connection->PlayerController) {
      UWorld *World = GetWorld();
      AGameModeBase *GameMode = World ? World->GetAuthGameMode() : nullptr;
      if (GameMode && GameMode->GameSession) {
        GameMode->GameSession->KickPlayer(
          Connection->PlayerController,
          FText::FromString(TEXT("Unexpected auth token"))
        );
      }
    }
    return;
  }

  APlayerController *PlayerController = Pending.PlayerController.Get();
  if (!IsValid(PlayerController)) {
    // Player disconnected between Login() and the Server RPC arriving — nothing to do.
    return;
  }

  if (Token.IsEmpty()) {
    UWorld *World = GetWorld();
    AGameModeBase *GameMode = World ? World->GetAuthGameMode() : nullptr;
    if (GameMode && GameMode->GameSession) {
      GameMode->GameSession->KickPlayer(
        PlayerController, FText::FromString(TEXT("Empty auth token"))
      );
    }
    return;
  }

  RunSidecarPlayerAuth(
    PlayerController, Pending.PlayerId, Pending.CharacterId, Token
  );
}

void URedwoodGameModeComponent::PruneStalePendingAuth() {
  if (PendingAuthByConnection.Num() == 0) {
    return;
  }

  const double Now = FPlatformTime::Seconds();
  UWorld *World = GetWorld();
  AGameModeBase *GameMode = World ? World->GetAuthGameMode() : nullptr;

  for (auto It = PendingAuthByConnection.CreateIterator(); It; ++It) {
    const FPendingAuth &Pending = It.Value();
    if (Now - Pending.CreatedAtSeconds <= PendingAuthTimeoutSeconds) {
      continue;
    }

    APlayerController *PC = Pending.PlayerController.Get();
    UE_LOG(
      LogRedwood,
      Warning,
      TEXT(
        "No auth-token Server RPC received for player %s within %.0fs; kicking"
      ),
      *Pending.PlayerId,
      PendingAuthTimeoutSeconds
    );
    if (IsValid(PC) && GameMode && GameMode->GameSession) {
      GameMode->GameSession->KickPlayer(
        PC, FText::FromString(TEXT("Auth token not received"))
      );
    }
    It.RemoveCurrent();
  }
}

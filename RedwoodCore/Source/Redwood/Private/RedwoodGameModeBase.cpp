// Copyright Incanta Games. All rights reserved.

#include "RedwoodGameModeBase.h"
#include "RedwoodGameplayTags.h"
#include "RedwoodPlayerController.h"
#include "RedwoodPlayerState.h"
#include "RedwoodServerGameSubsystem.h"

#if WITH_EDITOR
  #include "RedwoodEditorSettings.h"
#endif

#include "Dom/JsonObject.h"
#include "GameFramework/GameSession.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Guid.h"
#include "Net/OnlineEngineInterface.h"

#include "SocketIOClient.h"

void ARedwoodGameModeBase::InitGame(
  const FString &MapName, const FString &Options, FString &ErrorMessage
) {
  Super::InitGame(MapName, Options, ErrorMessage);

  bool bConnectToSidecar = false;

#if WITH_EDITOR
  URedwoodEditorSettings *RedwoodEditorSettings =
    GetMutableDefault<URedwoodEditorSettings>();
  bConnectToSidecar =
    !GIsEditor || RedwoodEditorSettings->bConnectToSidecarInPIE;
#else
  bConnectToSidecar = true;
#endif

  if (bConnectToSidecar) {
    Sidecar = ISocketIOClientModule::Get().NewValidNativePointer();

    URedwoodServerGameSubsystem *RedwoodServerGameSubsystem =
      GetGameInstance()->GetSubsystem<URedwoodServerGameSubsystem>();
    Sidecar->Connect(RedwoodServerGameSubsystem->SidecarUri);
  }

  FGameModeEvents::GameModeLogoutEvent.AddUObject(
    this, &ARedwoodGameModeBase::OnGameModeLogout
  );
}

void ARedwoodGameModeBase::OnGameModeLogout(
  AGameModeBase *GameMode, AController *Controller
) {
  APlayerController *PlayerController = Cast<APlayerController>(Controller);
  if (PlayerController == nullptr) {
    return;
  }

  UGameplayMessageSubsystem &MessageSubsystem =
    UGameplayMessageSubsystem::Get(this);
  MessageSubsystem.BroadcastMessage(
    TAG_Redwood_Player_Left, FRedwoodPlayerLeft{PlayerController}
  );
}

APlayerController *ARedwoodGameModeBase::Login(
  UPlayer *NewPlayer,
  ENetRole InRemoteRole,
  const FString &Portal,
  const FString &Options,
  const FUniqueNetIdRepl &UniqueId,
  FString &ErrorMessage
) {
  APlayerController *PlayerController = Super::Login(
    NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage
  );

  if (!ErrorMessage.IsEmpty() || PlayerController == nullptr) {
    return PlayerController;
  }

  bool bConnectToSidecar = false;

#if WITH_EDITOR
  URedwoodEditorSettings *RedwoodEditorSettings =
    GetMutableDefault<URedwoodEditorSettings>();
  bConnectToSidecar =
    !GIsEditor || RedwoodEditorSettings->bConnectToSidecarInPIE;
#else
  bConnectToSidecar = true;
#endif

  if (bConnectToSidecar) {
    if (UGameplayStatics::HasOption(Options, TEXT("RedwoodAuth"))) {
      FString PlayerId =
        UGameplayStatics::ParseOption(Options, TEXT("PlayerId"));
      FString CharacterId =
        UGameplayStatics::ParseOption(Options, TEXT("CharacterId"));
      FString Token = UGameplayStatics::ParseOption(Options, TEXT("Token"));

      // query for player legitimacy
      TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
      JsonObject->SetStringField(TEXT("playerId"), PlayerId);
      JsonObject->SetStringField(TEXT("characterId"), CharacterId);
      JsonObject->SetStringField(TEXT("token"), Token);

      Sidecar->Emit(
        TEXT("realm:servers:player-auth:game-server-to-sidecar"),
        JsonObject,
        [this, PlayerId, PlayerController](auto Response) {
          TSharedPtr<FJsonObject> MessageStruct = Response[0]->AsObject();
          FString Error = MessageStruct->GetStringField(TEXT("error"));

          if (Error.IsEmpty()) {
            TSharedPtr<FJsonObject> Character =
              MessageStruct->GetObjectField(TEXT("character"));
            FString CharacterId = Character->GetStringField(TEXT("id"));
            FString CharacterName = Character->GetStringField(TEXT("name"));

            FUniqueNetIdWrapper UniqueNetIdWrapper =
              UOnlineEngineInterface::Get()->CreateUniquePlayerIdWrapper(
                PlayerId + TEXT(":") + CharacterId, FName(TEXT("RedwoodMMO"))
              );
            FUniqueNetIdRepl UniqueId(UniqueNetIdWrapper.GetUniqueNetId());

            PlayerController->PlayerState->SetUniqueId(UniqueId);

            PlayerController->PlayerState->SetPlayerName(CharacterName);

            ARedwoodPlayerState *RedwoodPlayerState =
              Cast<ARedwoodPlayerState>(PlayerController->PlayerState);
            if (IsValid(RedwoodPlayerState)) {
              UE_LOG(
                LogRedwood,
                Log,
                TEXT("Player joined as character %s"),
                *CharacterId
              );

              RedwoodPlayerState->RedwoodCharacter =
                URedwoodClientInterface::ParseCharacter(Character);

              RedwoodPlayerState->OnRedwoodCharacterUpdated.Broadcast();

              RedwoodPlayerState->SetServerReady();

              HandleStartingNewPlayer(PlayerController);
            } else {
              UE_LOG(
                LogRedwood,
                Log,
                TEXT(
                  "Player joined as character %s, but we're not using RedwoodPlayerState"
                ),
                *CharacterId
              );
            }
          } else {
            // kick the player
            UE_LOG(
              LogRedwood,
              Error,
              TEXT("Player failed to authenticate, kicking them now: %s"),
              *Error
            );
            GameSession->KickPlayer(PlayerController, FText::FromString(Error));
          }
        }
      );
    } else {
      ErrorMessage =
        TEXT("Invalid authentication request: missing RedwoodAuth option");
    }
  }

  return PlayerController;
}

TArray<FString> ARedwoodGameModeBase::GetExpectedCharacterIds() const {
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

bool ARedwoodGameModeBase::PlayerCanRestart_Implementation(
  APlayerController *Player
) {
  ARedwoodPlayerState *RedwoodPlayerState =
    Cast<ARedwoodPlayerState>(Player->PlayerState);
  if (IsValid(RedwoodPlayerState)) {
    if (!RedwoodPlayerState->bServerReady) {
      return false;
    }
  }

  return Super::PlayerCanRestart_Implementation(Player);
}

void ARedwoodGameModeBase::FinishRestartPlayer(
  AController *NewPlayer, const FRotator &StartRotation
) {
  NewPlayer->Possess(NewPlayer->GetPawn());

  // If the Pawn is destroyed as part of possession we have to abort
  if (!IsValid(NewPlayer->GetPawn())) {
    FailedToRestartPlayer(NewPlayer);
  } else {
    ARedwoodPlayerState *RedwoodPlayerState =
      Cast<ARedwoodPlayerState>(NewPlayer->PlayerState);

    FRotator NewControlRotation = NewPlayer->GetPawn()->GetActorRotation();

    if (IsValid(RedwoodPlayerState)) {
      if (RedwoodPlayerState->RedwoodCharacter.Data) {
        USIOJsonObject *LastTransform =
          RedwoodPlayerState->RedwoodCharacter.Data->GetObjectField(
            TEXT("lastTransform")
          );
        if (IsValid(LastTransform)) {
          USIOJsonObject *ControlRotation =
            LastTransform->GetObjectField(TEXT("controlRotation"));
          if (ControlRotation) {
            float Roll = ControlRotation->GetNumberField(TEXT("x"));
            float Pitch = ControlRotation->GetNumberField(TEXT("y"));
            float Yaw = ControlRotation->GetNumberField(TEXT("z"));

            NewControlRotation = FRotator(Pitch, Yaw, Roll);
          }
        }
      }
    }

    ARedwoodPlayerController *RedwoodPlayerController =
      Cast<ARedwoodPlayerController>(NewPlayer);
    if (IsValid(RedwoodPlayerController)) {
      RedwoodPlayerController->bSkipPawnFaceRotation = true;
    }

    NewPlayer->ClientSetRotation(NewControlRotation, true);

    NewPlayer->SetControlRotation(NewControlRotation);

    SetPlayerDefaults(NewPlayer->GetPawn());

    K2_OnRestartPlayer(NewPlayer);
  }
}

APawn *ARedwoodGameModeBase::SpawnDefaultPawnAtTransform_Implementation(
  AController *NewPlayer, const FTransform &SpawnTransform
) {
  URedwoodServerGameSubsystem *RedwoodServerGameSubsystem =
    NewPlayer->GetWorld()
      ->GetGameInstance()
      ->GetSubsystem<URedwoodServerGameSubsystem>();

  ARedwoodPlayerState *RedwoodPlayerState =
    Cast<ARedwoodPlayerState>(NewPlayer->PlayerState);

  if (IsValid(RedwoodPlayerState)) {
    if (IsValid(RedwoodPlayerState->RedwoodCharacter.Data)) {
      USIOJsonObject *LastTransform =
        RedwoodPlayerState->RedwoodCharacter.Data->GetObjectField(
          TEXT("lastTransform")
        );
      if (IsValid(LastTransform)) {
        USIOJsonObject *Position =
          LastTransform->GetObjectField(TEXT("position"));
        USIOJsonObject *Rotation =
          LastTransform->GetObjectField(TEXT("rotation"));
        if (IsValid(Position) && IsValid(Rotation)) {
          float PosX = Position->GetNumberField(TEXT("x"));
          float PosY = Position->GetNumberField(TEXT("y"));
          float PosZ = Position->GetNumberField(TEXT("z"));

          float RotX = Rotation->GetNumberField(TEXT("x"));
          float RotY = Rotation->GetNumberField(TEXT("y"));
          float RotZ = Rotation->GetNumberField(TEXT("z"));

          FTransform Transform =
            FTransform(FRotator(RotX, RotY, RotZ), FVector(PosX, PosY, PosZ));

          return Super::SpawnDefaultPawnAtTransform_Implementation(
            NewPlayer, Transform
          );
        }
      }
    }

    UE_LOG(
      LogRedwood, Log, TEXT("No valid last transform found, using zone spawn")
    );

    // get all actors of the ARedwoodZoneSpawn class
    TArray<AActor *> ZoneSpawns;
    UGameplayStatics::GetAllActorsOfClass(
      NewPlayer->GetWorld(), ARedwoodZoneSpawn::StaticClass(), ZoneSpawns
    );

    for (AActor *ZoneSpawn : ZoneSpawns) {
      ARedwoodZoneSpawn *RedwoodZoneSpawn = Cast<ARedwoodZoneSpawn>(ZoneSpawn);
      if (IsValid(RedwoodZoneSpawn)) {
        if (RedwoodZoneSpawn->ZoneName == RedwoodServerGameSubsystem->ZoneName) {
          UE_LOG(
            LogRedwood,
            Log,
            TEXT("Found zone spawn for current zone %s"),
            *RedwoodZoneSpawn->ZoneName
          );

          return Super::SpawnDefaultPawnAtTransform_Implementation(
            NewPlayer, RedwoodZoneSpawn->GetSpawnTransform()
          );
        }
      }
    }
  }

  UE_LOG(
    LogRedwood,
    Error,
    TEXT(
      "Could not find a lastTransform for the character and there's no valid ARedwoodZoneSpawn found for this zone (%s). Using default transform."
    ),
    *RedwoodServerGameSubsystem->ZoneName
  );

  return Super::SpawnDefaultPawnAtTransform_Implementation(
    NewPlayer, SpawnTransform
  );
}

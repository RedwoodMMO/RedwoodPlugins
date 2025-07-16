// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodGameModeComponent.h"
#include "RedwoodCommonGameSubsystem.h"
#include "RedwoodGameplayTags.h"
#include "RedwoodPlayerState.h"
#include "RedwoodServerGameSubsystem.h"
#include "RedwoodZoneSpawn.h"

#if WITH_EDITOR
  #include "RedwoodEditorSettings.h"
#endif

#include "Dom/JsonObject.h"
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

  FTimerManager &TimerManager = GetWorld()->GetTimerManager();
  TimerManager.SetTimer(
    PostBeginPlayTimerHandle,
    this,
    &URedwoodGameModeComponent::PostBeginPlay,
    PostBeginPlayDelay,
    false
  );
}

void URedwoodGameModeComponent::PostBeginPlay() {
  URedwoodServerGameSubsystem *RedwoodServerGameSubsystem =
    GetWorld()->GetGameInstance()->GetSubsystem<URedwoodServerGameSubsystem>();

  if (RedwoodServerGameSubsystem) {
    RedwoodServerGameSubsystem->InitialDataLoad(
      FRedwoodDelegate::CreateLambda([this]() {
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
      })
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
  URedwoodServerGameSubsystem *RedwoodServerGameSubsystem =
    GetWorld()->GetGameInstance()->GetSubsystem<URedwoodServerGameSubsystem>();

  RedwoodServerGameSubsystem->FlushPersistence();
}

void URedwoodGameModeComponent::InitGame(
  const FString &MapName, const FString &Options, FString &ErrorMessage
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    Sidecar = ISocketIOClientModule::Get().NewValidNativePointer();

    URedwoodServerGameSubsystem *RedwoodServerGameSubsystem =
      GetWorld()->GetGameInstance()->GetSubsystem<URedwoodServerGameSubsystem>(
      );
    Sidecar->Connect(RedwoodServerGameSubsystem->SidecarUri);
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

  ARedwoodPlayerState *RedwoodPlayerState =
    Cast<ARedwoodPlayerState>(PlayerController->PlayerState);
  if (IsValid(RedwoodPlayerState)) {
    if (URedwoodCommonGameSubsystem::ShouldUseBackend(GameMode->GetWorld())) {
      if (Sidecar.IsValid() && Sidecar->bIsConnected) {
        TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
        JsonObject->SetStringField(
          TEXT("playerId"), RedwoodPlayerState->RedwoodCharacter.PlayerId
        );
        JsonObject->SetStringField(
          TEXT("characterId"), RedwoodPlayerState->RedwoodCharacter.Id
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
      Token = UGameplayStatics::ParseOption(Options, TEXT("Token"));
    }

    if (!PlayerId.IsEmpty() && !CharacterId.IsEmpty() && !Token.IsEmpty()) {
      // query for player legitimacy
      TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
      JsonObject->SetStringField(TEXT("playerId"), PlayerId);
      JsonObject->SetStringField(TEXT("characterId"), CharacterId);
      JsonObject->SetStringField(TEXT("token"), Token);

      if (!Sidecar.IsValid() || !Sidecar->bIsConnected) {
        ErrorMessage = TEXT("Sidecar is not connected");
        return PlayerController;
      }

      Sidecar->Emit(
        TEXT("realm:servers:player-auth:game-server-to-sidecar"),
        JsonObject,
        [this, PlayerId, PlayerController, GameMode](auto Response) {
          TSharedPtr<FJsonObject> MessageStruct = Response[0]->AsObject();
          FString Error = MessageStruct->GetStringField(TEXT("error"));

          if (Error.IsEmpty()) {
            TSharedPtr<FJsonObject> Character =
              MessageStruct->GetObjectField(TEXT("character"));
            FString CharacterId = Character->GetStringField(TEXT("id"));
            FString CharacterName = Character->GetStringField(TEXT("name"));

            ARedwoodPlayerState *RedwoodPlayerState =
              Cast<ARedwoodPlayerState>(PlayerController->PlayerState);
            if (IsValid(RedwoodPlayerState)) {
              UE_LOG(
                LogRedwood,
                Log,
                TEXT("Player joined as character %s"),
                *CharacterId
              );

              TSharedPtr<FJsonObject> Player =
                MessageStruct->GetObjectField(TEXT("player"));

              RedwoodPlayerState->RedwoodPlayerNickname =
                Player->GetStringField(TEXT("nickname"));

              const TSharedPtr<FJsonObject> *GuildObject;

              if (MessageStruct->TryGetObjectField(
                    TEXT("guild"), GuildObject
                  )) {
                RedwoodPlayerState->bRedwoodHasSelectedGuild = true;
                RedwoodPlayerState->RedwoodSelectedGuild =
                  URedwoodCommonGameSubsystem::ParseGuild(*GuildObject);
              }

              // This notifies subscribers to the OnRedwoodCharacterUpdated delegate (e.g. URedwoodCharacterComponent)
              RedwoodPlayerState->SetRedwoodCharacter(
                URedwoodCommonGameSubsystem::ParseCharacter(Character)
              );

              RedwoodPlayerState->SetServerReady();

              GameMode->HandleStartingNewPlayer(PlayerController);
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
            GameMode->GameSession->KickPlayer(
              PlayerController, FText::FromString(Error)
            );
          }
        }
      );
    } else {
      ErrorMessage =
        TEXT("Invalid authentication request: missing RedwoodAuth option");
    }
  } else {
    // we're likely PIE so just load the character from disk
    // based on the player controller index
    uint8 PlayerIndex = PlayerController->NetPlayerIndex;

    TArray<FRedwoodCharacterBackend> Characters =
      URedwoodCommonGameSubsystem::LoadAllCharactersFromDisk();

    if (PlayerIndex < Characters.Num()) {
      ARedwoodPlayerState *RedwoodPlayerState =
        Cast<ARedwoodPlayerState>(PlayerController->PlayerState);
      if (IsValid(RedwoodPlayerState)) {
        FRedwoodCharacterBackend &Character = Characters[PlayerIndex];
        RedwoodPlayerState->SetRedwoodCharacter(Character);

        RedwoodPlayerState->SetServerReady();

        GameMode->HandleStartingNewPlayer(PlayerController);
      } else {
        UE_LOG(
          LogRedwood,
          Log,
          TEXT("Can't load character data as we're not using RedwoodPlayerState"
          )
        );
      }
    } else {
      ErrorMessage = FString::Printf(
        TEXT("No character found for this player index %d"), PlayerIndex
      );
    }
  }

  return PlayerController;
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
  ARedwoodPlayerState *RedwoodPlayerState =
    Cast<ARedwoodPlayerState>(Player->PlayerState);
  if (IsValid(RedwoodPlayerState)) {
    if (!RedwoodPlayerState->bServerReady) {
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
    ARedwoodPlayerState *RedwoodPlayerState =
      Cast<ARedwoodPlayerState>(NewPlayer->PlayerState);

    FRotator NewControlRotation = NewPlayer->GetPawn()->GetActorRotation();

    if (IsValid(RedwoodPlayerState)) {
      if (RedwoodPlayerState->RedwoodCharacter.RedwoodData) {
        USIOJsonObject *LastLocation;
        if (RedwoodPlayerState->RedwoodCharacter.RedwoodData->TryGetObjectField(
              TEXT("lastLocation"), LastLocation
            )) {
          USIOJsonObject *LastTransform;

          if (LastLocation->TryGetObjectField(
                TEXT("transform"), LastTransform
              )) {
            USIOJsonObject *ControlRotation =
              LastTransform->GetObjectField(TEXT("controlRotation"));
            if (ControlRotation) {
              float Roll = ControlRotation->GetNumberField(TEXT("x"));
              float Pitch = ControlRotation->GetNumberField(TEXT("y"));
              float Yaw = ControlRotation->GetNumberField(TEXT("z"));

              NewControlRotation = FRotator(Pitch, Yaw, Roll);
            } else {
              UE_LOG(
                LogRedwood,
                Error,
                TEXT("Invalid lastTransform (no controlRotation object field)")
              );
            }
          }
        }
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

  TArray<ARedwoodZoneSpawn *> RedwoodZoneSpawns;
  for (AActor *ZoneSpawn : ZoneSpawns) {
    ARedwoodZoneSpawn *RedwoodZoneSpawn = Cast<ARedwoodZoneSpawn>(ZoneSpawn);
    if (IsValid(RedwoodZoneSpawn)) {
      if (RedwoodZoneSpawn->ZoneName == RedwoodServerGameSubsystem->ZoneName) {
        RedwoodZoneSpawns.Add(RedwoodZoneSpawn);
      }
    }
  }

  ARedwoodPlayerState *RedwoodPlayerState =
    Cast<ARedwoodPlayerState>(NewPlayer->PlayerState);

  if (IsValid(RedwoodPlayerState)) {
    if (IsValid(RedwoodPlayerState->RedwoodCharacter.RedwoodData)) {

      USIOJsonObject *LastLocation;
      if (RedwoodPlayerState->RedwoodCharacter.RedwoodData->TryGetObjectField(
            TEXT("lastLocation"), LastLocation
          )) {
        FString LastZoneName = LastLocation->GetStringField(TEXT("zoneName"));

        if (LastZoneName == RedwoodServerGameSubsystem->ZoneName) {

          FString LastSpawnName;
          USIOJsonObject *LastTransform;

          if (LastLocation->TryGetStringField(
                TEXT("spawnName"), LastSpawnName
              )) {
            for (ARedwoodZoneSpawn *ZoneSpawn : RedwoodZoneSpawns) {
              if (ZoneSpawn->SpawnName == LastSpawnName) {
                return ZoneSpawn->GetSpawnTransform();
              }
            }
          } else if (LastLocation->TryGetObjectField(
                       TEXT("transform"), LastTransform
                     )) {
            USIOJsonObject *Location =
              LastTransform->GetObjectField(TEXT("location"));
            USIOJsonObject *Rotation =
              LastTransform->GetObjectField(TEXT("rotation"));
            if (IsValid(Location) && IsValid(Rotation)) {
              float LocX = Location->GetNumberField(TEXT("x"));
              float LocY = Location->GetNumberField(TEXT("y"));
              float LocZ = Location->GetNumberField(TEXT("z"));

              float RotX = Rotation->GetNumberField(TEXT("x"));
              float RotY = Rotation->GetNumberField(TEXT("y"));
              float RotZ = Rotation->GetNumberField(TEXT("z"));

              FTransform Transform = FTransform(
                FRotator(RotY, RotZ, RotX), FVector(LocX, LocY, LocZ)
              );

              return Transform;
            } else {
              UE_LOG(
                LogRedwood,
                Error,
                TEXT(
                  "Invalid lastTransform (no location and/or rotation object fields)"
                )
              );
            }
          }
        }
      }
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
      return RedwoodZoneSpawns[0]->GetSpawnTransform();
    }
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

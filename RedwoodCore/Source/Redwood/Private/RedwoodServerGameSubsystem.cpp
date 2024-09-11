// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodServerGameSubsystem.h"
#include "RedwoodCharacter.h"
#include "RedwoodClientExecCommand.h"
#include "RedwoodCommonGameSubsystem.h"
#include "RedwoodGameModeAsset.h"
#include "RedwoodGameplayTags.h"
#include "RedwoodMapAsset.h"
#include "RedwoodSettings.h"

#if WITH_EDITOR
  #include "RedwoodEditorSettings.h"
#endif

#include "Engine/AssetManager.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/KismetStringLibrary.h"

#include "SocketIOClient.h"

void URedwoodServerGameSubsystem::Initialize(
  FSubsystemCollectionBase &Collection
) {
  Super::Initialize(Collection);

  UWorld *World = GetWorld();

  if (
    IsValid(World) &&
    (
      World->GetNetMode() == ENetMode::NM_DedicatedServer ||
      World->GetNetMode() == ENetMode::NM_ListenServer
    )
  ) {
    UE_LOG(
      LogRedwood,
      Log,
      TEXT("Initializing RedwoodServerGameSubsystem for server")
    );

    FPrimaryAssetType GameModeAssetType =
      URedwoodGameModeAsset::StaticClass()->GetFName();
    FPrimaryAssetType MapAssetType =
      URedwoodMapAsset::StaticClass()->GetFName();

    UAssetManager &AssetManager = UAssetManager::Get();

    UE_LOG(
      LogRedwood,
      Log,
      TEXT("Waiting for RedwoodGameModeAsset and RedwoodMapAsset to load")
    );

    // Load Redwood GameMode and Map assets so we can know which underlying GameMode and Map to load later
    TSharedPtr<FStreamableHandle> HandleModes =
      AssetManager.LoadPrimaryAssetsWithType(GameModeAssetType);
    TSharedPtr<FStreamableHandle> HandleMaps =
      AssetManager.LoadPrimaryAssetsWithType(MapAssetType);

    if (!HandleModes.IsValid() || !HandleMaps.IsValid()) {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT(
          "Failed to load RedwoodGameModeAsset or RedwoodMapAsset asset types; not initializing RedwoodServerGameSubsystem"
        )
      );
      return;
    }

    HandleModes->WaitUntilComplete();
    HandleMaps->WaitUntilComplete();

    TArray<UObject *> GameModesAssets;
    TArray<UObject *> MapsAssets;

    AssetManager.GetPrimaryAssetObjectList(GameModeAssetType, GameModesAssets);
    AssetManager.GetPrimaryAssetObjectList(MapAssetType, MapsAssets);

    for (UObject *Object : GameModesAssets) {
      URedwoodGameModeAsset *RedwoodGameMode =
        Cast<URedwoodGameModeAsset>(Object);
      if (ensure(RedwoodGameMode)) {
        if (RedwoodGameMode->GameModeType == ERedwoodGameModeType::GameModeBase) {
          GameModeClasses.Add(
            RedwoodGameMode->RedwoodId, RedwoodGameMode->GameModeBaseClass
          );
        } else {
          GameModeClasses.Add(
            RedwoodGameMode->RedwoodId, RedwoodGameMode->GameModeClass
          );
        }
      }
    }

    for (UObject *Object : MapsAssets) {
      URedwoodMapAsset *RedwoodMap = Cast<URedwoodMapAsset>(Object);
      if (ensure(RedwoodMap)) {
        Maps.Add(RedwoodMap->RedwoodId, RedwoodMap->MapId);
      }
    }

    UE_LOG(
      LogRedwood,
      Log,
      TEXT("Loaded %d GameMode assets and %d Map assets"),
      GameModeClasses.Num(),
      Maps.Num()
    );

    URedwoodSettings *RedwoodSettings = GetMutableDefault<URedwoodSettings>();
    if (RedwoodSettings->bServersAutoConnectToSidecar) {
#if WITH_EDITOR
      URedwoodEditorSettings *RedwoodEditorSettings =
        GetMutableDefault<URedwoodEditorSettings>();
      if (!GIsEditor || RedwoodEditorSettings->bConnectToSidecarInPIE) {
        InitializeSidecar();
      }
#else
      InitializeSidecar();
#endif
    }

    UGameplayMessageSubsystem &MessageSubsystem =
      UGameplayMessageSubsystem::Get(this);
    ListenerHandle = MessageSubsystem.RegisterListener(
      TAG_Redwood_Shutdown_Instance,
      this,
      &URedwoodServerGameSubsystem::OnShutdownMessage
    );

    UE_LOG(
      LogRedwood, Log, TEXT("Finished initializing RedwoodServerGameSubsystem")
    );
  }
}

void URedwoodServerGameSubsystem::OnShutdownMessage(
  FGameplayTag InChannel, const FRedwoodReason &Message
) {
  UE_LOG(
    LogRedwood,
    Log,
    TEXT("Received shutdown message, reason: %s"),
    *Message.Reason
  );
  bIsShuttingDown = true;
}

void URedwoodServerGameSubsystem::Deinitialize() {
  Super::Deinitialize();

  if (TimerHandle_UpdateSidecar.IsValid()) {
    GetGameInstance()->GetTimerManager().ClearTimer(TimerHandle_UpdateSidecar);
  }

  if (TimerHandle_UpdateSidecarLoading.IsValid()) {
    GetGameInstance()->GetTimerManager().ClearTimer(
      TimerHandle_UpdateSidecarLoading
    );
  }

  if (Sidecar.IsValid()) {
    ISocketIOClientModule::Get().ReleaseNativePointer(Sidecar);
    Sidecar = nullptr;
  }
}

void URedwoodServerGameSubsystem::InitializeSidecar() {
  Sidecar = ISocketIOClientModule::Get().NewValidNativePointer();

  Sidecar->OnEvent(
    TEXT("realm:servers:session:load-map"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      UE_LOG(LogRedwood, Log, TEXT("Received message to load a map"));
      const TSharedPtr<FJsonObject> *Object;

      TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject);

      if (Message->TryGetObject(Object) && Object) {
        UE_LOG(LogRedwood, Log, TEXT("LoadMap message is valid object"));
        TSharedPtr<FJsonObject> ActualObject = *Object;
        RequestId = ActualObject->GetStringField(TEXT("requestId"));
        Name = ActualObject->GetStringField(TEXT("name"));
        MapId = ActualObject->GetStringField(TEXT("mapId"));
        ModeId = ActualObject->GetStringField(TEXT("modeId"));
        bContinuousPlay = ActualObject->GetBoolField(TEXT("continuousPlay"));
        ActualObject->TryGetStringField(TEXT("password"), Password);
        ActualObject->TryGetStringField(TEXT("shortCode"), ShortCode);
        MaxPlayers = ActualObject->GetIntegerField(TEXT("maxPlayers"));
        TSharedPtr<FJsonObject> DataObj =
          ActualObject->GetObjectField(TEXT("data"));
        Data = NewObject<USIOJsonObject>();
        Data->SetRootObject(DataObj);
        ActualObject->TryGetStringField(TEXT("ownerPlayerId"), OwnerPlayerId);
        Channel = ActualObject->GetStringField(TEXT("channel"));

        UE_LOG(
          LogRedwood,
          Log,
          TEXT("LoadMap message has map (%s), mode (%s), and channel (%s)"),
          *MapId,
          *ModeId,
          *Channel
        );

        if (Channel.Contains(":")) {
          TArray<FString> ChannelParts;
          Channel.ParseIntoArray(ChannelParts, TEXT(":"), true);

          if (ChannelParts.Num() > 0) {
            ZoneName = ChannelParts[0];
          }

          if (ChannelParts.Num() > 1) {
            ShardName = ChannelParts[1];
          }
        }

        TSubclassOf<AGameModeBase> *GameModeToLoad =
          GameModeClasses.Find(ModeId);
        FPrimaryAssetId *MapToLoad = Maps.Find(MapId);

        if (GameModeToLoad == nullptr || MapToLoad == nullptr) {
          UE_LOG(
            LogRedwood, Error, TEXT("Failed to find GameMode or Map to load")
          );
          Response->SetStringField(
            TEXT("error"), TEXT("Invalid ModeId or MapId")
          );
          Sidecar->Emit(
            TEXT("realm:servers:session:load-map:response"), Response
          );
          return;
        }

        FString Error;
        FURL Url;
        Url.Protocol = "unreal";
        Url.Map = MapToLoad->PrimaryAssetName.ToString();

        // The preferred method to retrieve these variables is via this subsystem public
        // member variables, but we add them to the URL for convenience if needed
        Url.AddOption(*FString("requestId=" + RequestId));
        Url.AddOption(*FString("name=" + Name));
        Url.AddOption(*FString("mapId=" + MapId));
        Url.AddOption(*FString("modeId=" + ModeId));
        Url.AddOption(
          *FString("continuousPlay=" + FString::FromInt(bContinuousPlay))
        );
        Url.AddOption(*FString("password=" + Password));
        Url.AddOption(*FString("shortCode=" + ShortCode));
        Url.AddOption(*FString("maxPlayers=" + FString::FromInt(MaxPlayers)));
        Url.AddOption(*FString("ownerPlayerId=" + OwnerPlayerId));
        Url.AddOption(*FString("game=" + (*GameModeToLoad)->GetPathName()));

        TArray<FString> Keys;
        DataObj->Values.GetKeys(Keys);
        for (FString Key : Keys) {
          FString Value;
          if (DataObj->TryGetStringField(Key, Value)) {
            Url.AddOption(*FString(Key + "=" + Value));
          }
        }

        FString Command = FString::Printf(TEXT("open %s"), *Url.ToString());
        GetGameInstance()->GetEngine()->DeferredCommands.Add(Command);

        Response->SetStringField(TEXT("error"), TEXT(""));
      } else {
        UE_LOG(LogRedwood, Error, TEXT("LoadMap message is not valid object"));
        Response->SetStringField(TEXT("error"), TEXT("Invalid request"));
      }

      Sidecar->Emit(TEXT("realm:servers:session:load-map:response"), Response);
    }
  );

  Sidecar->OnConnectedCallback =
    [this](const FString &InSocketId, const FString &InSessionId) {
      if (Sidecar.IsValid()) {
        GetGameInstance()->GetTimerManager().SetTimer(
          TimerHandle_UpdateSidecarLoading,
          this,
          &URedwoodServerGameSubsystem::SendUpdateToSidecar,
          UpdateSidecarLoadingRate,
          true // loop
        );

        GetGameInstance()->GetTimerManager().SetTimer(
          TimerHandle_UpdateSidecar,
          this,
          &URedwoodServerGameSubsystem::SendUpdateToSidecar,
          UpdateSidecarRate,
          true, // loop
          0.f // immediately trigger first one
        );
      }
    };

  FString SidecarPort = TEXT("3020"); // default port
  FParse::Value(FCommandLine::Get(), TEXT("SidecarPort="), SidecarPort);

  SidecarUri = TEXT("ws://127.0.0.1:") + SidecarPort;
  UE_LOG(LogRedwood, Log, TEXT("Connecting to Sidecar at %s"), *SidecarUri);

  // Sidecar will always be on the same host
  Sidecar->Connect(SidecarUri);
}

void URedwoodServerGameSubsystem::SendUpdateToSidecar() {
  if (Sidecar.IsValid()) {
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

    UWorld *World = GetWorld();

    if (IsValid(World)) {
      AGameModeBase *GameMode = World->GetAuthGameMode();
      if (IsValid(GameMode)) {
        bool bWorldStarted = World->GetRealTimeSeconds() > 0;
        if (bWorldStarted) {
          if (TimerHandle_UpdateSidecarLoading.IsValid()) {
            GetGameInstance()->GetTimerManager().ClearTimer(
              TimerHandle_UpdateSidecarLoading
            );
          }

          if (bIsShuttingDown) {
            JsonObject->SetStringField(TEXT("state"), TEXT("Stopping"));
          } else {
            ARedwoodGameModeBase *RedwoodGameModeBase =
              Cast<ARedwoodGameModeBase>(GameMode);

            if (RedwoodGameModeBase) {
              // We're not inheriting from AGameMode so we're not match based (theoretically)
              // so we just assume we're in a started state
              JsonObject->SetStringField(TEXT("state"), TEXT("Started"));
            } else {
              if (GameMode->HasMatchStarted()) {
                if (GameMode->HasMatchEnded()) {
                  JsonObject->SetStringField(TEXT("state"), TEXT("Ended"));
                } else {
                  JsonObject->SetStringField(TEXT("state"), TEXT("Started"));
                }
              } else {
                JsonObject->SetStringField(
                  TEXT("state"), TEXT("WaitingForPlayers")
                );
              }
            }
          }
        }
      } else {
        JsonObject->SetStringField(TEXT("state"), TEXT("LoadingMap"));
      }

      JsonObject->SetNumberField(TEXT("numPlayers"), GameMode->GetNumPlayers());
    } else {
      bool bStarted = World->GetRealTimeSeconds() > 0;
      JsonObject->SetStringField(TEXT("state"), TEXT("LoadingMap"));

      JsonObject->SetNumberField(TEXT("numPlayers"), 0);
    }

    // We get these options from the URL instead of the member variables to
    // ensure the map got loaded

    JsonObject->SetStringField(
      TEXT("id"), World->URL.GetOption(TEXT("requestId="), TEXT(""))
    );

    JsonObject->SetStringField(
      TEXT("mapId"), World->URL.GetOption(TEXT("mapId="), TEXT(""))
    );

    JsonObject->SetStringField(
      TEXT("modeId"), World->URL.GetOption(TEXT("modeId="), TEXT(""))
    );

    Sidecar->Emit(TEXT("realm:servers:update-state"), JsonObject);
  }
}

void URedwoodServerGameSubsystem::CallExecCommandOnAllClients(
  const FString &Command
) {
  // spawn ARedwoodClientExecCommand
  UWorld *World = GetWorld();
  if (IsValid(World)) {
    // spawn actor
    ARedwoodClientExecCommand *ExecCommand = Cast<ARedwoodClientExecCommand>(
      UGameplayStatics::BeginDeferredActorSpawnFromClass(
        this, ARedwoodClientExecCommand::StaticClass(), FTransform()
      )
    );
    if (ExecCommand) {
      ExecCommand->Command = Command;
      UGameplayStatics::FinishSpawningActor(ExecCommand, FTransform());
    }
  }
}

void URedwoodServerGameSubsystem::TravelPlayerToZoneTransform(
  APlayerController *PlayerController,
  const FString &InZoneName,
  const FTransform &InTransform
) {
  FString UniqueId = PlayerController->PlayerState->GetUniqueId().ToString();

  FString PlayerId = UniqueId.Left(UniqueId.Find(TEXT(":")));
  FString CharacterId = UniqueId.RightChop(UniqueId.Find(TEXT(":")) + 1);

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);

  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("characterId"), CharacterId);
  Payload->SetStringField(TEXT("priorZoneName"), ZoneName);
  Payload->SetStringField(TEXT("zoneName"), InZoneName);

  TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());

  Payload->SetField(TEXT("spawnName"), NullValue);

  TSharedPtr<FJsonObject> Transform = MakeShareable(new FJsonObject);

  TSharedPtr<FJsonObject> Position = MakeShareable(new FJsonObject);
  Position->SetNumberField(TEXT("x"), InTransform.GetLocation().X);
  Position->SetNumberField(TEXT("y"), InTransform.GetLocation().Y);
  Position->SetNumberField(TEXT("z"), InTransform.GetLocation().Z);
  Transform->SetObjectField(TEXT("position"), Position);

  TSharedPtr<FJsonObject> Rotation = MakeShareable(new FJsonObject);
  auto RotationEuler = InTransform.GetRotation().Euler();
  Rotation->SetNumberField(TEXT("x"), RotationEuler.X);
  Rotation->SetNumberField(TEXT("y"), RotationEuler.Y);
  Rotation->SetNumberField(TEXT("z"), RotationEuler.Z);
  Transform->SetObjectField(TEXT("rotation"), Rotation);

  TSharedPtr<FJsonObject> ControlRotation = MakeShareable(new FJsonObject);
  ControlRotation->SetNumberField(
    TEXT("x"), PlayerController->GetControlRotation().Roll
  );
  ControlRotation->SetNumberField(
    TEXT("y"), PlayerController->GetControlRotation().Pitch
  );
  ControlRotation->SetNumberField(
    TEXT("z"), PlayerController->GetControlRotation().Yaw
  );
  Transform->SetObjectField(TEXT("controlRotation"), ControlRotation);

  Payload->SetObjectField(TEXT("transform"), Transform);

  if (Sidecar == nullptr || !Sidecar.IsValid() || !Sidecar->bIsConnected) {
    UE_LOG(
      LogRedwood,
      Error,
      TEXT("Sidecar is not connected; cannot travel player to new zone")
    );
    return;
  }

  UE_LOG(
    LogRedwood,
    Log,
    TEXT("Traveling player %s to zone %s"),
    *PlayerId,
    *InZoneName
  );

  Sidecar->Emit(
    TEXT("realm:servers:transfer-zone:game-server-to-sidecar"),
    Payload,
    [this, PlayerId, CharacterId, PlayerController](auto Response) {
      TSharedPtr<FJsonObject> MessageStruct = Response[0]->AsObject();
      FString Error = MessageStruct->GetStringField(TEXT("error"));

      if (!Error.IsEmpty()) {
        // kick the player
        UE_LOG(
          LogRedwood,
          Error,
          TEXT("Failed to transfer player to new zone, kicking them now: %s"),
          *Error
        );
        GetGameInstance()
          ->GetWorld()
          ->GetAuthGameMode()
          ->GameSession->KickPlayer(PlayerController, FText::FromString(Error));
      }
    }
  );
}

void URedwoodServerGameSubsystem::TravelPlayerToZoneSpawnName(
  APlayerController *PlayerController,
  const FString &InZoneName,
  const FString &InSpawnName
) {
  if (InSpawnName.IsEmpty()) {
    UE_LOG(
      LogRedwood,
      Error,
      TEXT("Cannot travel player to zone %s; provide a non-empty SpawnName"),
      *InZoneName
    );
    return;
  }

  FString UniqueId = PlayerController->PlayerState->GetUniqueId().ToString();

  FString PlayerId = UniqueId.Left(UniqueId.Find(TEXT(":")));
  FString CharacterId = UniqueId.RightChop(UniqueId.Find(TEXT(":")) + 1);

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);

  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("characterId"), CharacterId);
  Payload->SetStringField(TEXT("priorZoneName"), ZoneName);
  Payload->SetStringField(TEXT("zoneName"), InZoneName);

  TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());

  Payload->SetStringField(TEXT("spawnName"), InSpawnName);
  Payload->SetField(TEXT("transform"), NullValue);

  if (Sidecar == nullptr || !Sidecar.IsValid() || !Sidecar->bIsConnected) {
    UE_LOG(
      LogRedwood,
      Error,
      TEXT("Sidecar is not connected; cannot travel player to new zone")
    );
    return;
  }

  UE_LOG(
    LogRedwood,
    Log,
    TEXT("Traveling player %s to zone %s"),
    *PlayerId,
    *InZoneName
  );

  Sidecar->Emit(
    TEXT("realm:servers:transfer-zone:game-server-to-sidecar"),
    Payload,
    [this, PlayerId, CharacterId, PlayerController](auto Response) {
      TSharedPtr<FJsonObject> MessageStruct = Response[0]->AsObject();
      FString Error = MessageStruct->GetStringField(TEXT("error"));

      if (!Error.IsEmpty()) {
        // kick the player
        UE_LOG(
          LogRedwood,
          Error,
          TEXT("Failed to transfer player to new zone, kicking them now: %s"),
          *Error
        );
        GetGameInstance()
          ->GetWorld()
          ->GetAuthGameMode()
          ->GameSession->KickPlayer(PlayerController, FText::FromString(Error));
      }
    }
  );
}

void URedwoodServerGameSubsystem::FlushPlayerCharacterData() {
  UWorld *World = GetWorld();
  if (!IsValid(World)) {
    UE_LOG(
      LogRedwood,
      Error,
      TEXT("Can't FlushPlayerCharacterData: World is not valid")
    );
    return;
  }

  AGameStateBase *GameState = World->GetGameState();
  if (!IsValid(GameState)) {
    UE_LOG(
      LogRedwood,
      Error,
      TEXT("Can't FlushPlayerCharacterData: GameState is not valid")
    );
    return;
  }

  TArray<TObjectPtr<APlayerState>> PlayerArray = GameState->PlayerArray;

  if (PlayerArray.Num() == 0) {
    return;
  }

  if (Sidecar && Sidecar->bIsConnected) {
    TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());

    TArray<TSharedPtr<FJsonValue>> CharactersArray;
    for (TObjectPtr<APlayerState> PlayerState : GameState->PlayerArray) {
      ARedwoodPlayerState *RedwoodPlayerState =
        Cast<ARedwoodPlayerState>(PlayerState);
      if (RedwoodPlayerState) {
        APawn *Pawn = PlayerState->GetPawn();
        if (Pawn) {
          ARedwoodCharacter *RedwoodCharacter = Cast<ARedwoodCharacter>(Pawn);
          if (RedwoodCharacter) {
            if (
            !RedwoodCharacter->IsCharacterCreatorDataDirty() &&
            !RedwoodCharacter->IsMetadataDirty() &&
            !RedwoodCharacter->IsEquippedInventoryDirty() &&
            !RedwoodCharacter->IsNonequippedInventoryDirty() &&
            !RedwoodCharacter->IsDataDirty()
            ) {
              continue;
            }

            UE_LOG(
              LogRedwood,
              Log,
              TEXT("Flushing character %s"),
              *RedwoodCharacter->RedwoodCharacterName
            );

            TSharedPtr<FJsonObject> CharacterObject =
              MakeShareable(new FJsonObject);
            CharacterObject->SetStringField(
              TEXT("playerId"), RedwoodCharacter->RedwoodPlayerId
            );
            CharacterObject->SetStringField(
              TEXT("characterId"), RedwoodCharacter->RedwoodCharacterId
            );

            if (RedwoodCharacter->IsCharacterCreatorDataDirty()) {
              USIOJsonObject *CharacterCreatorData =
                RedwoodCharacter->SerializeBackendData(
                  TEXT("CharacterCreatorData")
                );
              if (CharacterCreatorData) {
                RedwoodPlayerState->RedwoodCharacter.CharacterCreatorData =
                  CharacterCreatorData;
                CharacterObject->SetObjectField(
                  TEXT("characterCreatorData"),
                  CharacterCreatorData->GetRootObject()
                );
              }
            }

            if (RedwoodCharacter->IsMetadataDirty()) {
              USIOJsonObject *Metadata =
                RedwoodCharacter->SerializeBackendData(TEXT("Metadata"));
              if (Metadata) {
                RedwoodPlayerState->RedwoodCharacter.Metadata = Metadata;
                CharacterObject->SetObjectField(
                  TEXT("metadata"), Metadata->GetRootObject()
                );
              }
            }

            if (RedwoodCharacter->IsEquippedInventoryDirty()) {
              USIOJsonObject *EquippedInventory =
                RedwoodCharacter->SerializeBackendData(TEXT("EquippedInventory")
                );
              if (EquippedInventory) {
                RedwoodPlayerState->RedwoodCharacter.EquippedInventory =
                  EquippedInventory;
                CharacterObject->SetObjectField(
                  TEXT("equippedInventory"), EquippedInventory->GetRootObject()
                );
              }
            }

            if (RedwoodCharacter->IsNonequippedInventoryDirty()) {
              USIOJsonObject *NonequippedInventory =
                RedwoodCharacter->SerializeBackendData(
                  TEXT("NonequippedInventory")
                );
              if (NonequippedInventory) {
                RedwoodPlayerState->RedwoodCharacter.NonequippedInventory =
                  NonequippedInventory;
                CharacterObject->SetObjectField(
                  TEXT("nonequippedInventory"),
                  NonequippedInventory->GetRootObject()
                );
              }
            }

            if (RedwoodCharacter->IsDataDirty()) {
              USIOJsonObject *CharData =
                RedwoodCharacter->SerializeBackendData(TEXT("Data"));
              if (CharData) {
                RedwoodPlayerState->RedwoodCharacter.Data = CharData;
                CharacterObject->SetObjectField(
                  TEXT("data"), CharData->GetRootObject()
                );
              }
            }

            TSharedPtr<FJsonValueObject> Value =
              MakeShareable(new FJsonValueObject(CharacterObject));
            CharactersArray.Add(Value);

            RedwoodCharacter->ClearDirtyFlags();
          } else {
            UE_LOG(
              LogRedwood,
              Error,
              TEXT("Pawn %s is not a RedwoodCharacter"),
              *Pawn->GetName()
            );
          }
        } else {
          UE_LOG(
            LogRedwood,
            Error,
            TEXT("PlayerState %s has no pawn"),
            *PlayerState->GetPlayerName()
          );
        }
      } else {
        UE_LOG(
          LogRedwood, Error, TEXT("PlayerState is not a RedwoodPlayerState")
        );
      }
    }

    if (CharactersArray.Num() == 0) {
      UE_LOG(LogRedwood, Log, TEXT("No characters to flush"));
      return;
    }

    UE_LOG(
      LogRedwood, Log, TEXT("Flushing %d characters"), CharactersArray.Num()
    );

    Payload->SetArrayField(TEXT("characters"), CharactersArray);
    Payload->SetStringField(TEXT("id"), TEXT("game-server"));

    Sidecar->Emit(TEXT("realm:characters:set:server"), Payload);
  } else {
    // save the dirty characters to disk

    FString SavePath = FPaths::ProjectSavedDir() / TEXT("Characters");
    FPaths::NormalizeDirectoryName(SavePath);

    if (!FPaths::DirectoryExists(SavePath)) {
      IFileManager::Get().MakeDirectory(*SavePath, true);
    }

    for (TObjectPtr<APlayerState> PlayerState : GameState->PlayerArray) {
      ARedwoodPlayerState *RedwoodPlayerState =
        Cast<ARedwoodPlayerState>(PlayerState);
      if (RedwoodPlayerState) {
        APawn *Pawn = PlayerState->GetPawn();
        if (Pawn) {
          ARedwoodCharacter *RedwoodCharacter = Cast<ARedwoodCharacter>(Pawn);
          if (RedwoodCharacter) {
            if (
            !RedwoodCharacter->IsCharacterCreatorDataDirty() &&
            !RedwoodCharacter->IsMetadataDirty() &&
            !RedwoodCharacter->IsEquippedInventoryDirty() &&
            !RedwoodCharacter->IsNonequippedInventoryDirty() &&
            !RedwoodCharacter->IsDataDirty()
            ) {
              continue;
            }

            UE_LOG(
              LogRedwood,
              Log,
              TEXT("Flushing character %s to disk"),
              *RedwoodCharacter->RedwoodCharacterName
            );

            // since we save the whole json to disk, we update all
            // of the data here to ensure proper variable name serialization

            USIOJsonObject *CharacterCreatorData =
              RedwoodCharacter->SerializeBackendData(TEXT("CharacterCreatorData"
              ));
            if (CharacterCreatorData) {
              RedwoodPlayerState->RedwoodCharacter.CharacterCreatorData =
                CharacterCreatorData;
            }

            USIOJsonObject *Metadata =
              RedwoodCharacter->SerializeBackendData(TEXT("Metadata"));
            if (Metadata) {
              RedwoodPlayerState->RedwoodCharacter.Metadata = Metadata;
            }

            USIOJsonObject *EquippedInventory =
              RedwoodCharacter->SerializeBackendData(TEXT("EquippedInventory"));
            if (EquippedInventory) {
              RedwoodPlayerState->RedwoodCharacter.EquippedInventory =
                EquippedInventory;
            }

            USIOJsonObject *NonequippedInventory =
              RedwoodCharacter->SerializeBackendData(TEXT("NonequippedInventory"
              ));
            if (NonequippedInventory) {
              RedwoodPlayerState->RedwoodCharacter.NonequippedInventory =
                NonequippedInventory;
            }

            USIOJsonObject *CharData =
              RedwoodCharacter->SerializeBackendData(TEXT("Data"));
            if (CharData) {
              RedwoodPlayerState->RedwoodCharacter.Data = CharData;
            }

            URedwoodCommonGameSubsystem::SaveCharacterToDisk(
              RedwoodPlayerState->RedwoodCharacter
            );
          }
        }
      }
    }
  }
}
// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodServerGameSubsystem.h"
#include "RedwoodCharacterComponent.h"
#include "RedwoodClientExecCommand.h"
#include "RedwoodCommonGameSubsystem.h"
#include "RedwoodGameModeAsset.h"
#include "RedwoodGameplayTags.h"
#include "RedwoodMapAsset.h"
#include "RedwoodPlayerState.h"
#include "RedwoodSettings.h"
#include "RedwoodSyncComponent.h"
#include "RedwoodSyncItemAsset.h"
#include "Types/RedwoodTypesPersistence.h"

#if WITH_EDITOR
  #include "RedwoodEditorSettings.h"
#endif

#include "Engine/AssetManager.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "JsonObjectConverter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"

#include "SIOJsonValue.h"
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
      TEXT("Waiting for RedwoodGameModeAsset amd RedwoodMapAsset to load")
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
        if (!RedwoodGameMode->RedwoodId.IsEmpty()) {
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
    }

    for (UObject *Object : MapsAssets) {
      URedwoodMapAsset *RedwoodMap = Cast<URedwoodMapAsset>(Object);
      if (ensure(RedwoodMap)) {
        if (!RedwoodMap->RedwoodId.IsEmpty()) {
          Maps.Add(RedwoodMap->RedwoodId, RedwoodMap->MapId);
        }
      }
    }

    UE_LOG(
      LogRedwood,
      Log,
      TEXT("Loaded %d GameMode assets and %d Map assets"),
      GameModeClasses.Num(),
      Maps.Num()
    );

    FPrimaryAssetType SyncItemAssetType =
      URedwoodSyncItemAsset::StaticClass()->GetFName();
    TSharedPtr<FStreamableHandle> HandleSyncItems =
      AssetManager.LoadPrimaryAssetsWithType(SyncItemAssetType);

    if (HandleSyncItems.IsValid()) {
      UE_LOG(LogRedwood, Log, TEXT("Waiting for RedwoodSyncItemAsset to load"));

      HandleSyncItems->WaitUntilComplete();
      TArray<UObject *> SyncItemAssets;
      AssetManager.GetPrimaryAssetObjectList(SyncItemAssetType, SyncItemAssets);

      for (UObject *Object : SyncItemAssets) {
        URedwoodSyncItemAsset *RedwoodSyncItem =
          Cast<URedwoodSyncItemAsset>(Object);
        if (ensure(RedwoodSyncItem)) {
          if (!RedwoodSyncItem->RedwoodTypeId.IsEmpty()) {
            SyncItemTypesByTypeId.Add(
              RedwoodSyncItem->RedwoodTypeId, RedwoodSyncItem
            );
            SyncItemTypesByPrimaryAssetId.Add(
              RedwoodSyncItem->GetPrimaryAssetId().ToString(), RedwoodSyncItem
            );
          }
        }
      }

      UE_LOG(
        LogRedwood, Log, TEXT("Loaded %d SyncItem assets"), SyncItemAssets.Num()
      );
    } else {
      UE_LOG(
        LogRedwood,
        Warning,
        TEXT(
          "No RedwoodSyncItemAsset asset type in the Asset Manager; continuing without loading"
        )
      );
    }

    URedwoodSettings *RedwoodSettings = GetMutableDefault<URedwoodSettings>();
    if (RedwoodSettings->bServersAutoConnectToSidecar) {
      if (URedwoodCommonGameSubsystem::ShouldUseBackend(World)) {
        InitializeSidecar();
      }
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
        const TSharedPtr<FJsonObject> *DataObj;
        Data = NewObject<USIOJsonObject>();
        if (ActualObject->TryGetObjectField(TEXT("data"), DataObj)) {
          Data->SetRootObject(*DataObj);
        } else {
          Data->SetRootObject(MakeShareable(new FJsonObject));
        }
        ActualObject->TryGetStringField(TEXT("ownerPlayerId"), OwnerPlayerId);
        Channel = ActualObject->GetStringField(TEXT("channel"));
        ActualObject->TryGetStringField(TEXT("parentProxyId"), ParentProxyId);

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
        Data->GetRootObject()->Values.GetKeys(Keys);
        for (FString Key : Keys) {
          FString Value;
          if (Data->GetRootObject()->TryGetStringField(Key, Value)) {
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

#if WITH_EDITOR
    if (World->WorldType == EWorldType::PIE) {
      // we skip loading the level in PIE, so we just use the options
      // that got set earlier and assuming we're already in the correct
      // level
      JsonObject->SetStringField(TEXT("id"), RequestId);
      JsonObject->SetStringField(TEXT("mapId"), MapId);
      JsonObject->SetStringField(TEXT("modeId"), ModeId);
    } else
#endif
    {
      JsonObject->SetStringField(
        TEXT("id"), World->URL.GetOption(TEXT("requestId="), TEXT(""))
      );

      JsonObject->SetStringField(
        TEXT("mapId"), World->URL.GetOption(TEXT("mapId="), TEXT(""))
      );

      JsonObject->SetStringField(
        TEXT("modeId"), World->URL.GetOption(TEXT("modeId="), TEXT(""))
      );
    }

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
  const FTransform &InTransform,
  const FString &OptionalProxyId
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

  if (!OptionalProxyId.IsEmpty()) {
    Payload->SetStringField(TEXT("proxyId"), OptionalProxyId);
  } else {
    Payload->SetField(TEXT("proxyId"), NullValue);
  }

  Payload->SetField(TEXT("spawnName"), NullValue);

  TSharedPtr<FJsonObject> Transform = MakeShareable(new FJsonObject);

  TSharedPtr<FJsonObject> Location = MakeShareable(new FJsonObject);
  Location->SetNumberField(TEXT("x"), InTransform.GetLocation().X);
  Location->SetNumberField(TEXT("y"), InTransform.GetLocation().Y);
  Location->SetNumberField(TEXT("z"), InTransform.GetLocation().Z);
  Transform->SetObjectField(TEXT("location"), Location);

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
  const FString &InSpawnName,
  const FString &OptionalProxyId
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

  if (!OptionalProxyId.IsEmpty()) {
    Payload->SetStringField(TEXT("proxyId"), OptionalProxyId);
  } else {
    Payload->SetField(TEXT("proxyId"), NullValue);
  }

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

void URedwoodServerGameSubsystem::FlushPersistence() {
  FlushPlayerCharacterData();
  FlushZoneData();
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

  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());

    TArray<TSharedPtr<FJsonValue>> CharactersArray;
    for (TObjectPtr<APlayerState> PlayerState : GameState->PlayerArray) {
      ARedwoodPlayerState *RedwoodPlayerState =
        Cast<ARedwoodPlayerState>(PlayerState);
      if (RedwoodPlayerState) {
        APawn *Pawn = PlayerState->GetPawn();
        if (Pawn) {
          URedwoodCharacterComponent *CharacterComponent =
            Pawn->GetComponentByClass<URedwoodCharacterComponent>();
          if (CharacterComponent) {
            if (
            !CharacterComponent->IsCharacterCreatorDataDirty() &&
            !CharacterComponent->IsMetadataDirty() &&
            !CharacterComponent->IsEquippedInventoryDirty() &&
            !CharacterComponent->IsNonequippedInventoryDirty() &&
            !CharacterComponent->IsDataDirty()
            ) {
              continue;
            }

            UE_LOG(
              LogRedwood,
              Log,
              TEXT("Flushing character %s"),
              *CharacterComponent->RedwoodCharacterName
            );

            TSharedPtr<FJsonObject> CharacterObject =
              MakeShareable(new FJsonObject);
            CharacterObject->SetStringField(
              TEXT("playerId"), CharacterComponent->RedwoodPlayerId
            );
            CharacterObject->SetStringField(
              TEXT("characterId"), CharacterComponent->RedwoodCharacterId
            );

            if (CharacterComponent->IsCharacterCreatorDataDirty()) {
              USIOJsonObject *CharacterCreatorData =
                URedwoodCommonGameSubsystem::SerializeBackendData(
                  CharacterComponent->bStoreDataInActor
                    ? (UObject *)Pawn
                    : (UObject *)CharacterComponent,
                  CharacterComponent->CharacterCreatorDataVariableName
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

            if (CharacterComponent->IsMetadataDirty()) {
              USIOJsonObject *Metadata =
                URedwoodCommonGameSubsystem::SerializeBackendData(
                  CharacterComponent->bStoreDataInActor
                    ? (UObject *)Pawn
                    : (UObject *)CharacterComponent,
                  CharacterComponent->MetadataVariableName
                );
              if (Metadata) {
                RedwoodPlayerState->RedwoodCharacter.Metadata = Metadata;
                CharacterObject->SetObjectField(
                  TEXT("metadata"), Metadata->GetRootObject()
                );
              }
            }

            if (CharacterComponent->IsEquippedInventoryDirty()) {
              USIOJsonObject *EquippedInventory =
                URedwoodCommonGameSubsystem::SerializeBackendData(
                  CharacterComponent->bStoreDataInActor
                    ? (UObject *)Pawn
                    : (UObject *)CharacterComponent,
                  CharacterComponent->EquippedInventoryVariableName
                );
              if (EquippedInventory) {
                RedwoodPlayerState->RedwoodCharacter.EquippedInventory =
                  EquippedInventory;
                CharacterObject->SetObjectField(
                  TEXT("equippedInventory"), EquippedInventory->GetRootObject()
                );
              }
            }

            if (CharacterComponent->IsNonequippedInventoryDirty()) {
              USIOJsonObject *NonequippedInventory =
                URedwoodCommonGameSubsystem::SerializeBackendData(
                  CharacterComponent->bStoreDataInActor
                    ? (UObject *)Pawn
                    : (UObject *)CharacterComponent,
                  CharacterComponent->NonequippedInventoryVariableName
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

            if (CharacterComponent->IsDataDirty()) {
              USIOJsonObject *CharData =
                URedwoodCommonGameSubsystem::SerializeBackendData(
                  CharacterComponent->bStoreDataInActor
                    ? (UObject *)Pawn
                    : (UObject *)CharacterComponent,
                  CharacterComponent->DataVariableName
                );
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

            CharacterComponent->ClearDirtyFlags();
          } else {
            UE_LOG(
              LogRedwood,
              Error,
              TEXT("Pawn %s doesn't have a RedwoodCharacterComponent"),
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

    Payload->SetArrayField(TEXT("characters"), CharactersArray);
    Payload->SetStringField(TEXT("id"), TEXT("game-server"));

    Sidecar->Emit(TEXT("realm:characters:set:server"), Payload);
  } else {
    // save the dirty characters to disk

    FString SavePath =
      FPaths::ProjectSavedDir() / TEXT("Persistence") / TEXT("Characters");
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
          URedwoodCharacterComponent *CharacterComponent =
            Pawn->GetComponentByClass<URedwoodCharacterComponent>();
          if (CharacterComponent) {
            if (
            !CharacterComponent->IsCharacterCreatorDataDirty() &&
            !CharacterComponent->IsMetadataDirty() &&
            !CharacterComponent->IsEquippedInventoryDirty() &&
            !CharacterComponent->IsNonequippedInventoryDirty() &&
            !CharacterComponent->IsDataDirty()
            ) {
              continue;
            }

            // since we save the whole json to disk, we update all
            // of the data here to ensure proper variable name serialization

            if (CharacterComponent->bUseCharacterCreatorData) {
              USIOJsonObject *CharacterCreatorData =
                URedwoodCommonGameSubsystem::SerializeBackendData(
                  CharacterComponent->bStoreDataInActor
                    ? (UObject *)Pawn
                    : (UObject *)CharacterComponent,
                  CharacterComponent->CharacterCreatorDataVariableName
                );
              if (CharacterCreatorData) {
                RedwoodPlayerState->RedwoodCharacter.CharacterCreatorData =
                  CharacterCreatorData;
              }
            }

            if (CharacterComponent->bUseMetadata) {
              USIOJsonObject *Metadata =
                URedwoodCommonGameSubsystem::SerializeBackendData(
                  CharacterComponent->bStoreDataInActor
                    ? (UObject *)Pawn
                    : (UObject *)CharacterComponent,
                  CharacterComponent->MetadataVariableName
                );
              if (Metadata) {
                RedwoodPlayerState->RedwoodCharacter.Metadata = Metadata;
              }
            }

            if (CharacterComponent->bUseEquippedInventory) {
              USIOJsonObject *EquippedInventory =
                URedwoodCommonGameSubsystem::SerializeBackendData(
                  CharacterComponent->bStoreDataInActor
                    ? (UObject *)Pawn
                    : (UObject *)CharacterComponent,
                  CharacterComponent->EquippedInventoryVariableName
                );
              if (EquippedInventory) {
                RedwoodPlayerState->RedwoodCharacter.EquippedInventory =
                  EquippedInventory;
              }
            }

            if (CharacterComponent->bUseNonequippedInventory) {
              USIOJsonObject *NonequippedInventory =
                URedwoodCommonGameSubsystem::SerializeBackendData(
                  CharacterComponent->bStoreDataInActor
                    ? (UObject *)Pawn
                    : (UObject *)CharacterComponent,
                  CharacterComponent->NonequippedInventoryVariableName
                );
              if (NonequippedInventory) {
                RedwoodPlayerState->RedwoodCharacter.NonequippedInventory =
                  NonequippedInventory;
              }
            }

            if (CharacterComponent->bUseData) {
              USIOJsonObject *CharData =
                URedwoodCommonGameSubsystem::SerializeBackendData(
                  CharacterComponent->bStoreDataInActor
                    ? (UObject *)Pawn
                    : (UObject *)CharacterComponent,
                  CharacterComponent->DataVariableName
                );
              if (CharData) {
                RedwoodPlayerState->RedwoodCharacter.Data = CharData;
              }
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

void URedwoodServerGameSubsystem::InitialDataLoad(FRedwoodDelegate OnComplete) {
  InitialDataLoadCompleteDelegate = OnComplete;

  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    if (Sidecar == nullptr || !Sidecar.IsValid() || !Sidecar->bIsConnected) {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT("Sidecar is not connected; cannot load initial data")
      );
      return;
    }

    TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
    Sidecar->Emit(
      TEXT("realm:servers:session:persistence:initial-load"),
      Payload,
      [this](auto Response) {
        TSharedPtr<FJsonObject> MessageStruct = Response[0]->AsObject();
        FString Error = MessageStruct->GetStringField(TEXT("error"));

        if (!Error.IsEmpty()) {
          UE_LOG(
            LogRedwood, Error, TEXT("Failed to load initial data: %s"), *Error
          );
        } else {
          UE_LOG(LogRedwood, Log, TEXT("Loaded initial data"));
          PostInitialDataLoad(MessageStruct);
        }
      }
    );

  } else {
    // load from disk
    FString SavePath =
      FPaths::ProjectSavedDir() / TEXT("Persistence") / TEXT("Maps");
    FPaths::NormalizeDirectoryName(SavePath);

    if (!FPaths::DirectoryExists(SavePath)) {
      IFileManager::Get().MakeDirectory(*SavePath, true);
    }

    UWorld *World = GetWorld();
    if (!IsValid(World)) {
      UE_LOG(
        LogRedwood, Error, TEXT("Can't InitialDataLoad: World is not valid")
      );
      return;
    }

    FString MapName = World->GetMapName();

    FString MapSavePath = SavePath / MapName + TEXT(".json");

    if (!FPaths::FileExists(MapSavePath)) {
      UE_LOG(LogRedwood, Log, TEXT("No saved data for map %s"), *MapName);
      return;
    }

    FString JsonString;
    FFileHelper::LoadFileToString(JsonString, *MapSavePath);

    TSharedPtr<FJsonObject> ZoneJsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    if (!FJsonSerializer::Deserialize(Reader, ZoneJsonObject)) {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT("Failed to deserialize JSON for map %s"),
        *MapName
      );
      return;
    }

    PostInitialDataLoad(ZoneJsonObject);
  }
}

void URedwoodServerGameSubsystem::PostInitialDataLoad(
  TSharedPtr<FJsonObject> ZoneJsonObject
) {
  FRedwoodZoneData InitialLoad =
    URedwoodCommonGameSubsystem::ParseZoneData(ZoneJsonObject);

  UWorld *World = GetWorld();
  if (!IsValid(World)) {
    UE_LOG(
      LogRedwood, Error, TEXT("Can't PostInitialDataLoad: World is not valid")
    );
    return;
  }

  AGameStateBase *GameState = World->GetGameState();
  if (!IsValid(GameState)) {
    UE_LOG(
      LogRedwood,
      Error,
      TEXT("Can't PostInitialDataLoad: GameState is not valid")
    );
    return;
  }

  URedwoodSyncComponent *GameStateSync =
    GameState->GetComponentByClass<URedwoodSyncComponent>();

  if (InitialLoad.Data && GameStateSync) {
    URedwoodSyncItemAsset *ProxyItem =
      SyncItemTypesByTypeId.FindRef(TEXT("proxy"));
    if (ProxyItem) {
      URedwoodCommonGameSubsystem::DeserializeBackendData(
        GameStateSync,
        InitialLoad.Data,
        ProxyItem->DataVariableName,
        ProxyItem->LatestSchemaVersion
      );
    } else {
      UE_LOG(
        LogRedwood,
        Warning,
        TEXT("No 'proxy' sync item type found, can't load world data")
      );
    }
  }

  for (FRedwoodSyncItem &Item : InitialLoad.PersistentItems) {
    UpdateSyncItem(Item);
  }

  InitialDataLoadCompleteDelegate.ExecuteIfBound();
}

void URedwoodServerGameSubsystem::UpdateSyncItem(FRedwoodSyncItem &Item) {
  UWorld *World = GetWorld();
  if (!IsValid(World)) {
    UE_LOG(LogRedwood, Error, TEXT("Can't UpdateSyncItem: World is not valid"));
    return;
  }

  URedwoodSyncComponent *SyncItemComponent;

  SyncItemComponent = SyncItemComponentsById.FindRef(Item.Id);

  URedwoodSyncItemAsset *ItemType = SyncItemTypesByTypeId.FindRef(Item.TypeId);

  if (ItemType == nullptr) {
    UE_LOG(
      LogRedwood,
      Error,
      TEXT(
        "Can't spawn SyncItemComponent of type %s because it's not registered"
      ),
      *Item.TypeId
    );
    return;
  }

  if (SyncItemComponent == nullptr) {
    // spawn it
    TSoftClassPtr<AActor> ActorClass = ItemType->ActorClass;
    if (ActorClass.IsNull()) {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT(
          "Can't spawn SyncItemComponent of type %s because it has no ActorClass"
        ),
        *Item.TypeId
      );
      return;
    }

    AActor *Actor = World->SpawnActor<AActor>(ActorClass.Get());

    if (Actor == nullptr) {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT("Failed to spawn SyncItemComponent of type %s"),
        *Item.TypeId
      );
      return;
    }

    SyncItemComponent = Actor->GetComponentByClass<URedwoodSyncComponent>();

    if (SyncItemComponent == nullptr) {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT(
          "Spawned actor for SyncItemComponent of type %s, but the actor has no RedwoodSyncComponent"
        ),
        *Item.TypeId
      );
      return;
    }

    SyncItemComponent->RedwoodId = Item.Id;
    SyncItemComponent->SkipInitialSave();

    SyncItemComponentsById.Add(Item.Id, SyncItemComponent);
  }

  AActor *Actor = SyncItemComponent->GetOwner();
  USceneComponent *ActorRootComponent = Actor->GetRootComponent();

  if (ActorRootComponent == nullptr) {
    UE_LOG(
      LogRedwood,
      Error,
      TEXT("SyncItemComponent %s has no root component; can't update transform"
      ),
      *Item.Id
    );
    return;
  }

  ActorRootComponent->SetWorldTransform(Item.Transform);

  if (Item.Data) {
    URedwoodCommonGameSubsystem::DeserializeBackendData(
      SyncItemComponent->bStoreDataInActor ? (UObject *)Actor
                                           : (UObject *)SyncItemComponent,
      Item.Data,
      ItemType->DataVariableName,
      ItemType->LatestSchemaVersion
    );
  }
}

void URedwoodServerGameSubsystem::FlushZoneData() {
  TSharedPtr<FJsonObject> ZoneData = MakeShareable(new FJsonObject);

  bool bZoneDataDirty = false;

  UWorld *World = GetWorld();
  if (!IsValid(World)) {
    UE_LOG(LogRedwood, Error, TEXT("Can't FlushZoneData: World is not valid"));
    return;
  }

  AGameStateBase *GameState = World->GetGameState();
  if (!IsValid(GameState)) {
    UE_LOG(
      LogRedwood, Error, TEXT("Can't FlushZoneData: GameState is not valid")
    );
    return;
  }

  URedwoodSyncComponent *GameStatePersistence =
    GameState->GetComponentByClass<URedwoodSyncComponent>();

  if (GameStatePersistence) {
    URedwoodSyncItemAsset *ProxyItem =
      SyncItemTypesByTypeId.FindRef(TEXT("proxy"));
    if (ProxyItem) {
      if (
        !URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld()) ||
        GameStatePersistence->IsDataDirty() ||
        GameStatePersistence->ShouldDoInitialSave()
      ) {
        // always add the data if we're not using the backend

        if (GameStatePersistence->IsDataDirty()) {
          bZoneDataDirty = true;
          GameStatePersistence->ClearDirtyFlags();
        }

        if (GameStatePersistence->ShouldDoInitialSave()) {
          bZoneDataDirty = true;
          GameStatePersistence->SkipInitialSave();
        }

        USIOJsonObject *DataObject =
          URedwoodCommonGameSubsystem::SerializeBackendData(
            GameStatePersistence, ProxyItem->DataVariableName
          );
        ZoneData->SetObjectField(TEXT("data"), DataObject->GetRootObject());
      }
    } else {
      UE_LOG(
        LogRedwood,
        Warning,
        TEXT("No 'proxy' sync item type found, can't save world data")
      );
    }
  }

  // get data from SyncItemComponentsById
  TArray<TSharedPtr<FJsonValue>> PersistentItemsArray;
  for (auto &Pair : SyncItemComponentsById) {
    URedwoodSyncComponent *SyncItemComponent = Pair.Value;

    if (SyncItemComponent->RedwoodId == TEXT("proxy")) {
      // don't save the proxy/world data here
      continue;
    }

    FPrimaryAssetId AssetId = SyncItemComponent->SyncItemType;
    if (!AssetId.IsValid()) {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT("SyncItemComponent %s has no PrimaryAssetId"),
        *SyncItemComponent->GetName()
      );
      continue;
    }

    // load the data from the SyncItemAsset from the asset manager
    URedwoodSyncItemAsset *SyncItemType =
      SyncItemTypesByPrimaryAssetId.FindRef(AssetId.ToString());

    if (SyncItemType == nullptr) {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT("Couldn't find SyncItemComponent %s PrimaryAssetId in map"),
        *SyncItemComponent->GetName()
      );
      continue;
    }

    bool bUpdateItem = SyncItemComponent->IsTransformDirty() ||
      SyncItemComponent->IsDataDirty() ||
      SyncItemComponent->ShouldDoInitialSave();

    if (!URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld()) || bUpdateItem) {
      // always add the data if we're not using the backend

      if (bUpdateItem) {
        bZoneDataDirty = true;
      }

      TSharedPtr<FJsonObject> ItemObject = MakeShareable(new FJsonObject());

      ItemObject->SetStringField(TEXT("id"), SyncItemComponent->RedwoodId);
      ItemObject->SetStringField(
        TEXT("typeId"), SyncItemType->RedwoodTypeId
      );

      // This flag should be redundant as we already checked the bools above
      // but we'll keep it just for now.
      bool bItemShouldBeSaved = false;

      if (
        !URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld()) ||
        SyncItemComponent->IsTransformDirty() ||
        SyncItemComponent->ShouldDoInitialSave()
      ) {
        bItemShouldBeSaved = true;

        FTransform Transform = SyncItemComponent->GetOwner()->GetTransform();
        FVector Location = Transform.GetLocation();
        FVector Rotation = Transform.GetRotation().Euler();
        FVector Scale = Transform.GetScale3D();

        TSharedPtr<FJsonObject> TransformObject =
          MakeShareable(new FJsonObject());
        TSharedPtr<FJsonObject> LocationObject =
          MakeShareable(new FJsonObject());
        TSharedPtr<FJsonObject> RotationObject =
          MakeShareable(new FJsonObject());
        TSharedPtr<FJsonObject> ScaleObject = MakeShareable(new FJsonObject());
        LocationObject->SetNumberField(TEXT("x"), Location.X);
        LocationObject->SetNumberField(TEXT("y"), Location.Y);
        LocationObject->SetNumberField(TEXT("z"), Location.Z);
        RotationObject->SetNumberField(TEXT("x"), Rotation.X);
        RotationObject->SetNumberField(TEXT("y"), Rotation.Y);
        RotationObject->SetNumberField(TEXT("z"), Rotation.Z);
        ScaleObject->SetNumberField(TEXT("x"), Scale.X);
        ScaleObject->SetNumberField(TEXT("y"), Scale.Y);
        ScaleObject->SetNumberField(TEXT("z"), Scale.Z);
        TransformObject->SetObjectField(TEXT("location"), LocationObject);
        TransformObject->SetObjectField(TEXT("rotation"), RotationObject);
        TransformObject->SetObjectField(TEXT("scale"), ScaleObject);
        ItemObject->SetObjectField(TEXT("transform"), TransformObject);
      }

      if (
        !URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld()) ||
        SyncItemComponent->IsDataDirty() ||
        SyncItemComponent->ShouldDoInitialSave()
      ) {
        bItemShouldBeSaved = true;

        USIOJsonObject *DataObject =
          URedwoodCommonGameSubsystem::SerializeBackendData(
            SyncItemComponent->bStoreDataInActor
              ? (UObject *)SyncItemComponent->GetOwner()
              : (UObject *)SyncItemComponent,
            SyncItemType->DataVariableName
          );
        ItemObject->SetObjectField(TEXT("data"), DataObject->GetRootObject());
      }

      if (SyncItemComponent->ShouldDoInitialSave()) {
        SyncItemComponent->SkipInitialSave();
      }

      SyncItemComponent->ClearDirtyFlags();

      if (bItemShouldBeSaved) {
        TSharedPtr<FJsonValueObject> Value =
          MakeShareable(new FJsonValueObject(ItemObject));
        PersistentItemsArray.Add(Value);
      }
    }
  }

  ZoneData->SetArrayField(TEXT("persistentItems"), PersistentItemsArray);

  if (bZoneDataDirty) {
    if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
      if (Sidecar == nullptr || !Sidecar.IsValid() || !Sidecar->bIsConnected) {
        UE_LOG(
          LogRedwood,
          Error,
          TEXT("Sidecar is not connected; cannot flush zone data")
        );
        return;
      }

      Sidecar->Emit(
        TEXT("realm:servers:session:persistence:update-zone"), ZoneData
      );
    } else {
      // save to disk

      UE_LOG(LogRedwood, Log, TEXT("Saving zone data to disk"));

      FString SavePath =
        FPaths::ProjectSavedDir() / TEXT("Persistence") / TEXT("Maps");
      FPaths::NormalizeDirectoryName(SavePath);

      if (!FPaths::DirectoryExists(SavePath)) {
        IFileManager::Get().MakeDirectory(*SavePath, true);
      }

      FString MapName = World->GetMapName();
      FString MapSavePath = SavePath / MapName + TEXT(".json");

      FString JsonString;
      TSharedRef<TJsonWriter<>> Writer =
        TJsonWriterFactory<>::Create(&JsonString);
      FJsonSerializer::Serialize(ZoneData.ToSharedRef(), Writer);

      FFileHelper::SaveStringToFile(JsonString, *MapSavePath);
    }
  }
}

void URedwoodServerGameSubsystem::RegisterSyncComponent(
  URedwoodSyncComponent *InComponent
) {
  if (InComponent == nullptr) {
    UE_LOG(
      LogRedwood, Error, TEXT("Can't register null URedwoodSyncComponent")
    );
    return;
  }

  if (SyncItemComponentsById.Find(InComponent->RedwoodId) != nullptr) {
    return;
  }

  if (InComponent->RedwoodId.IsEmpty()) {
    UE_LOG(
      LogRedwood,
      Error,
      TEXT("Can't register URedwoodSyncComponent '%s' with empty RedwoodId"),
      *InComponent->GetName()
    );
    return;
  }

  SyncItemComponentsById.Add(InComponent->RedwoodId, InComponent);
}

void URedwoodServerGameSubsystem::PutBlob(
  const FString &Key,
  const TArray<uint8> &Value,
  FRedwoodErrorOutputDelegate OnComplete
) {
  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);

  Payload->SetStringField(TEXT("id"), "game-server");
  Payload->SetStringField(TEXT("key"), Key);

  TSharedPtr<FJsonValueBinary> BinaryValue =
    MakeShareable(new FJsonValueBinary(Value));
  Payload->SetField(TEXT("blob"), BinaryValue);

  Sidecar->Emit(TEXT("realm:blobs:put"), Payload, [OnComplete](auto Response) {
    TSharedPtr<FJsonObject> MessageStruct = Response[0]->AsObject();
    FString Error = MessageStruct->GetStringField(TEXT("error"));

    OnComplete.ExecuteIfBound(Error);
  });
}

void URedwoodServerGameSubsystem::GetBlob(
  const FString &Key, FRedwoodGetBlobOutputDelegate OnComplete
) {
  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);

  Payload->SetStringField(TEXT("id"), "game-server");
  Payload->SetStringField(TEXT("key"), Key);

  Sidecar
    ->Emit(TEXT("realm:blobs:get"), Payload, [this, OnComplete](auto Response) {
      TSharedPtr<FJsonObject> MessageStruct = Response[0]->AsObject();
      FString Error = MessageStruct->GetStringField(TEXT("error"));

      if (!Error.IsEmpty()) {
        FRedwoodGetBlobOutput Output;
        Output.Error = Error;
        OnComplete.ExecuteIfBound(Output);
        return;
      }

      FRedwoodGetBlobOutput Output;

      TSharedPtr<FJsonValue> Value = MessageStruct->TryGetField(TEXT("blob"));
      if (Value.IsValid() && FJsonValueBinary::IsBinary(Value)) {
        Output.Blob = FJsonValueBinary::AsBinary(Value);
      } else {
        Output.Error = TEXT("Failed to parse blob");
      }

      OnComplete.ExecuteIfBound(Output);
    });
}

void URedwoodServerGameSubsystem::PutSaveGame(
  const FString &Key, USaveGame *Value, FRedwoodErrorOutputDelegate OnComplete
) {
  TSharedRef<TArray<uint8>> ObjectBytes(new TArray<uint8>());

  if (UGameplayStatics::SaveGameToMemory(Value, *ObjectBytes) && (ObjectBytes->Num() > 0)) {
    PutBlob(Key, *ObjectBytes, OnComplete);
  } else {
    OnComplete.ExecuteIfBound(TEXT("Failed to serialize SaveGame"));
  }
}

void URedwoodServerGameSubsystem::GetSaveGame(
  const FString &Key, FRedwoodGetSaveGameOutputDelegate OnComplete
) {
  GetBlob(
    Key,
    FRedwoodGetBlobOutputDelegate::CreateLambda(
      [this, OnComplete](FRedwoodGetBlobOutput Response) {
        FRedwoodGetSaveGameOutput Output;

        if (!Response.Error.IsEmpty()) {
          Output.Error = Response.Error;
          Output.SaveGame = nullptr;
          OnComplete.ExecuteIfBound(Output);
          return;
        }

        if (Response.Blob.Num() > 0) {
          Output.SaveGame = UGameplayStatics::LoadGameFromMemory(Response.Blob);
        }

        OnComplete.ExecuteIfBound(Output);
      }
    )
  );
}

void URedwoodServerGameSubsystem::RequestEngineExit(bool bForce) {
  FGenericPlatformMisc::RequestExit(bForce);
}

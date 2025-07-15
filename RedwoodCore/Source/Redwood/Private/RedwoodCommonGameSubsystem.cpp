// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodCommonGameSubsystem.h"

#if WITH_EDITOR
  #include "RedwoodEditorSettings.h"
#endif

#include "SIOJConvert.h"
#include "SIOJsonObject.h"

void URedwoodCommonGameSubsystem::Initialize(
  FSubsystemCollectionBase &Collection
) {
  Super::Initialize(Collection);
}

void URedwoodCommonGameSubsystem::Deinitialize() {
  Super::Deinitialize();
}

void URedwoodCommonGameSubsystem::SaveCharacterToDisk(
  FRedwoodCharacterBackend &Character
) {
  TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

  JsonObject->SetStringField(TEXT("id"), Character.Id);
  JsonObject->SetStringField(TEXT("createdAt"), Character.CreatedAt.ToString());
  JsonObject->SetStringField(TEXT("updatedAt"), Character.UpdatedAt.ToString());

  if (Character.bArchived) {
    JsonObject->SetStringField(
      TEXT("archivedAt"), Character.ArchivedAt.ToString()
    );
  } else {
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull);
    JsonObject->SetField(TEXT("archivedAt"), NullValue);
  }

  JsonObject->SetStringField(TEXT("playerId"), Character.PlayerId);
  JsonObject->SetStringField(TEXT("name"), Character.Name);
  if (Character.CharacterCreatorData) {
    JsonObject->SetObjectField(
      TEXT("characterCreatorData"),
      Character.CharacterCreatorData->GetRootObject()
    );
  }
  if (Character.Metadata) {
    JsonObject->SetObjectField(
      TEXT("metadata"), Character.Metadata->GetRootObject()
    );
  }
  if (Character.EquippedInventory) {
    JsonObject->SetObjectField(
      TEXT("equippedInventory"), Character.EquippedInventory->GetRootObject()
    );
  }
  if (Character.NonequippedInventory) {
    JsonObject->SetObjectField(
      TEXT("nonequippedInventory"),
      Character.NonequippedInventory->GetRootObject()
    );
  }
  if (Character.Data) {
    JsonObject->SetObjectField(TEXT("data"), Character.Data->GetRootObject());
  }

  FString OutputString;
  TSharedRef<TJsonWriter<TCHAR>> JsonWriter =
    TJsonWriterFactory<TCHAR>::Create(&OutputString);
  FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

  FString SavePath =
    FPaths::ProjectSavedDir() / TEXT("Persistence") / TEXT("Characters");
  FPaths::NormalizeFilename(SavePath);
  FString FileName = SavePath / (Character.Id + TEXT(".json"));

  FFileHelper::SaveStringToFile(OutputString, *FileName);
}

TArray<FRedwoodCharacterBackend>
URedwoodCommonGameSubsystem::LoadAllCharactersFromDisk() {
  FString SavePath =
    FPaths::ProjectSavedDir() / TEXT("Persistence") / TEXT("Characters");
  FPaths::NormalizeFilename(SavePath);

  TArray<FString> Files;
  IFileManager::Get().FindFiles(Files, *SavePath, TEXT("json"));

  TArray<FRedwoodCharacterBackend> Characters;

  for (FString File : Files) {
    Characters.Add(LoadCharacterFromDisk(FPaths::GetBaseFilename(File)));
  }

  return Characters;
}

FRedwoodCharacterBackend URedwoodCommonGameSubsystem::LoadCharacterFromDisk(
  FString CharacterId
) {
  FString SavePath =
    FPaths::ProjectSavedDir() / TEXT("Persistence") / TEXT("Characters");
  FPaths::NormalizeFilename(SavePath);

  FString FilePath = SavePath / CharacterId + TEXT(".json");
  FString FileContents;
  FFileHelper::LoadFileToString(FileContents, *FilePath);

  TSharedPtr<FJsonObject> JsonObject;
  TSharedRef<TJsonReader<TCHAR>> JsonReader =
    TJsonReaderFactory<TCHAR>::Create(FileContents);
  if (!FJsonSerializer::Deserialize(JsonReader, JsonObject)) {
    return FRedwoodCharacterBackend();
  }

  FRedwoodCharacterBackend Character;

  Character.Id = JsonObject->GetStringField(TEXT("id"));
  FDateTime::Parse(
    JsonObject->GetStringField(TEXT("createdAt")), Character.CreatedAt
  );
  FDateTime::Parse(
    JsonObject->GetStringField(TEXT("updatedAt")), Character.UpdatedAt
  );

  FString ArchivedAt;
  if (JsonObject->TryGetStringField(TEXT("archivedAt"), ArchivedAt)) {
    FDateTime::Parse(ArchivedAt, Character.ArchivedAt);
    Character.bArchived = true;
  }

  Character.PlayerId = JsonObject->GetStringField(TEXT("playerId"));
  Character.Name = JsonObject->GetStringField(TEXT("name"));

  TSharedPtr<FJsonObject> CharacterCreatorDataObj =
    JsonObject->GetObjectField(TEXT("characterCreatorData"));
  if (CharacterCreatorDataObj.IsValid()) {
    Character.CharacterCreatorData = NewObject<USIOJsonObject>();
    Character.CharacterCreatorData->SetRootObject(CharacterCreatorDataObj);
  }

  TSharedPtr<FJsonObject> MetadataObj =
    JsonObject->GetObjectField(TEXT("metadata"));
  if (MetadataObj.IsValid()) {
    Character.Metadata = NewObject<USIOJsonObject>();
    Character.Metadata->SetRootObject(MetadataObj);
  }

  TSharedPtr<FJsonObject> EquippedInventoryObj =
    JsonObject->GetObjectField(TEXT("equippedInventory"));
  if (EquippedInventoryObj.IsValid()) {
    Character.EquippedInventory = NewObject<USIOJsonObject>();
    Character.EquippedInventory->SetRootObject(EquippedInventoryObj);
  }

  TSharedPtr<FJsonObject> NonequippedInventoryObj =
    JsonObject->GetObjectField(TEXT("nonequippedInventory"));
  if (NonequippedInventoryObj.IsValid()) {
    Character.NonequippedInventory = NewObject<USIOJsonObject>();
    Character.NonequippedInventory->SetRootObject(NonequippedInventoryObj);
  }

  TSharedPtr<FJsonObject> DataObj = JsonObject->GetObjectField(TEXT("data"));
  if (DataObj.IsValid()) {
    Character.Data = NewObject<USIOJsonObject>();
    Character.Data->SetRootObject(DataObj);
  }

  return Character;
}

uint8 URedwoodCommonGameSubsystem::GetCharactersOnDiskCount() {
  FString SavePath =
    FPaths::ProjectSavedDir() / TEXT("Persistence") / TEXT("Characters");
  FPaths::NormalizeFilename(SavePath);

  TArray<FString> Files;
  IFileManager::Get().FindFiles(Files, *SavePath, TEXT("json"));

  return Files.Num();
}

FRedwoodCharacterBackend URedwoodCommonGameSubsystem::ParseCharacter(
  TSharedPtr<FJsonObject> CharacterObj
) {
  FRedwoodCharacterBackend Character;
  Character.Id = CharacterObj->GetStringField(TEXT("id"));

  FDateTime::ParseIso8601(
    *CharacterObj->GetStringField(TEXT("createdAt")), Character.CreatedAt
  );

  FDateTime::ParseIso8601(
    *CharacterObj->GetStringField(TEXT("updatedAt")), Character.UpdatedAt
  );

  FString ArchivedAt;
  if (CharacterObj->TryGetStringField(TEXT("archivedAt"), ArchivedAt)) {
    FDateTime::ParseIso8601(*ArchivedAt, Character.ArchivedAt);
    Character.bArchived = true;
  }

  Character.PlayerId = CharacterObj->GetStringField(TEXT("playerId"));

  Character.Name = CharacterObj->GetStringField(TEXT("name"));

  const TSharedPtr<FJsonObject> *CharacterCharacterCreatorData = nullptr;
  if (CharacterObj->TryGetObjectField(
        TEXT("characterCreatorData"), CharacterCharacterCreatorData
      )) {
    Character.CharacterCreatorData = NewObject<USIOJsonObject>();
    Character.CharacterCreatorData->SetRootObject(*CharacterCharacterCreatorData
    );
  }

  const TSharedPtr<FJsonObject> *CharacterMetadata = nullptr;
  if (CharacterObj->TryGetObjectField(TEXT("metadata"), CharacterMetadata)) {
    Character.Metadata = NewObject<USIOJsonObject>();
    Character.Metadata->SetRootObject(*CharacterMetadata);
  }

  const TSharedPtr<FJsonObject> *CharacterEquippedInventory = nullptr;
  if (CharacterObj->TryGetObjectField(
        TEXT("equippedInventory"), CharacterEquippedInventory
      )) {
    Character.EquippedInventory = NewObject<USIOJsonObject>();
    Character.EquippedInventory->SetRootObject(*CharacterEquippedInventory);
  }

  const TSharedPtr<FJsonObject> *NonequippedInventory = nullptr;
  if (CharacterObj->TryGetObjectField(
        TEXT("nonequippedInventory"), NonequippedInventory
      )) {
    Character.NonequippedInventory = NewObject<USIOJsonObject>();
    Character.NonequippedInventory->SetRootObject(*NonequippedInventory);
  }

  const TSharedPtr<FJsonObject> *CharacterData = nullptr;
  if (CharacterObj->TryGetObjectField(TEXT("data"), CharacterData)) {
    Character.Data = NewObject<USIOJsonObject>();
    Character.Data->SetRootObject(*CharacterData);
  }

  const TSharedPtr<FJsonObject> *RedwoodData = nullptr;
  if (CharacterObj->TryGetObjectField(TEXT("redwoodData"), RedwoodData)) {
    Character.RedwoodData = NewObject<USIOJsonObject>();
    Character.RedwoodData->SetRootObject(*RedwoodData);
  }

  return Character;
}

FRedwoodGameServerProxy URedwoodCommonGameSubsystem::ParseServerProxy(
  TSharedPtr<FJsonObject> ServerProxy
) {
  FRedwoodGameServerProxy Server;

  Server.Id = ServerProxy->GetStringField(TEXT("id"));

  FDateTime::ParseIso8601(
    *ServerProxy->GetStringField(TEXT("createdAt")), Server.CreatedAt
  );

  FDateTime::ParseIso8601(
    *ServerProxy->GetStringField(TEXT("updatedAt")), Server.UpdatedAt
  );

  FString EndedAt;
  if (ServerProxy->TryGetStringField(TEXT("endedAt"), EndedAt)) {
    FDateTime::ParseIso8601(*EndedAt, Server.EndedAt);
    Server.bEnded = true;
  }

  // Start common properties for GameServerInstance loading details

  Server.Name = ServerProxy->GetStringField(TEXT("name"));

  Server.ModeId = ServerProxy->GetStringField(TEXT("modeId"));

  ServerProxy->TryGetStringField(TEXT("mapId"), Server.MapId);

  Server.bContinuousPlay = ServerProxy->GetBoolField(TEXT("continuousPlay"));

  ServerProxy->TryGetStringField(TEXT("password"), Server.Password);

  ServerProxy->TryGetStringField(TEXT("shortCode"), Server.ShortCode);

  Server.MaxPlayersPerShard =
    ServerProxy->GetIntegerField(TEXT("maxPlayersPerShard"));

  const TSharedPtr<FJsonObject> *Data;
  if (ServerProxy->TryGetObjectField(TEXT("data"), Data)) {
    USIOJsonObject *DataObject = NewObject<USIOJsonObject>();
    DataObject->SetRootObject(*Data);
    Server.Data = DataObject;
  }

  ServerProxy->TryGetStringField(TEXT("ownerPlayerId"), Server.OwnerPlayerId);

  // End common properties for GameServerInstance loading details

  Server.Region = ServerProxy->GetStringField(TEXT("region"));

  Server.bStartOnBoot = ServerProxy->GetBoolField(TEXT("startOnBoot"));

  FString ZonesCSV;
  if (ServerProxy->TryGetStringField(TEXT("zones"), ZonesCSV)) {
    ZonesCSV.ParseIntoArray(Server.Zones, TEXT(","), true);
  }

  ServerProxy->TryGetNumberField(
    TEXT("numPlayersToAddShard"), Server.NumPlayersToAddShard
  );

  ServerProxy->TryGetNumberField(
    TEXT("numMinutesToDestroyEmptyShard"), Server.NumMinutesToDestroyEmptyShard
  );

  Server.bPublic = ServerProxy->GetBoolField(TEXT("public"));

  Server.bProxyEndsWhenCollectionEnds =
    ServerProxy->GetBoolField(TEXT("proxyEndsWhenCollectionEnds"));

  Server.CurrentPlayers = ServerProxy->GetIntegerField(TEXT("currentPlayers"));

  ServerProxy->TryGetBoolField(TEXT("hasPassword"), Server.bHasPassword);

  ServerProxy->TryGetStringField(
    TEXT("activeCollectionId"), Server.ActiveCollectionId
  );

  return Server;
}

FRedwoodGameServerInstance URedwoodCommonGameSubsystem::ParseServerInstance(
  TSharedPtr<FJsonObject> ServerInstance
) {
  FRedwoodGameServerInstance Instance;

  Instance.Id = ServerInstance->GetStringField(TEXT("id"));

  FDateTime::ParseIso8601(
    *ServerInstance->GetStringField(TEXT("createdAt")), Instance.CreatedAt
  );

  FDateTime::ParseIso8601(
    *ServerInstance->GetStringField(TEXT("updatedAt")), Instance.UpdatedAt
  );

  Instance.ProviderId = ServerInstance->GetStringField(TEXT("providerId"));

  FString StartedAt;
  if (ServerInstance->TryGetStringField(TEXT("startedAt"), StartedAt)) {
    FDateTime::ParseIso8601(*StartedAt, Instance.StartedAt);
  }

  FString EndedAt;
  if (ServerInstance->TryGetStringField(TEXT("endedAt"), EndedAt)) {
    FDateTime::ParseIso8601(*EndedAt, Instance.EndedAt);
    Instance.bEnded = true;
  }

  ServerInstance->TryGetStringField(TEXT("connection"), Instance.Connection);

  ServerInstance->TryGetStringField(TEXT("channel"), Instance.Channel);

  if (Instance.Channel.Contains(":")) {
    TArray<FString> ChannelParts;
    Instance.Channel.ParseIntoArray(ChannelParts, TEXT(":"), true);

    if (ChannelParts.Num() > 0) {
      Instance.ZoneName = ChannelParts[0];
    }

    if (ChannelParts.Num() > 1) {
      Instance.ShardName = ChannelParts[1];
    }
  }

  Instance.ContainerId = ServerInstance->GetStringField(TEXT("containerId"));

  Instance.CollectionId = ServerInstance->GetStringField(TEXT("collectionId"));

  return Instance;
}

FRedwoodZoneData URedwoodCommonGameSubsystem::ParseZoneData(
  TSharedPtr<FJsonObject> ZoneData
) {
  FRedwoodZoneData Data;

  const TSharedPtr<FJsonObject> *DataObj;
  if (ZoneData->TryGetObjectField(TEXT("data"), DataObj)) {
    Data.Data = NewObject<USIOJsonObject>();
    Data.Data->SetRootObject(*DataObj);
  }

  const TArray<TSharedPtr<FJsonValue>> *PersistentItems;
  if (ZoneData->TryGetArrayField(TEXT("items"), PersistentItems)) {
    for (const TSharedPtr<FJsonValue> &Item : *PersistentItems) {
      if (Item->Type == EJson::Object) {
        TSharedPtr<FJsonObject> ItemObj = Item->AsObject();
        FRedwoodSyncItem SyncItem = ParseSyncItem(ItemObj);
        Data.Items.Add(SyncItem);
      }
    }
  }

  return Data;
}

FRedwoodSyncItem URedwoodCommonGameSubsystem::ParseSyncItem(
  TSharedPtr<FJsonObject> SyncItem
) {
  FRedwoodSyncItem Item;

  TSharedPtr<FJsonObject> StateObj = SyncItem->GetObjectField(TEXT("state"));
  Item.State = ParseSyncItemState(StateObj);

  TSharedPtr<FJsonObject> MovementObj =
    SyncItem->GetObjectField(TEXT("movement"));
  Item.Movement = ParseSyncItemMovement(MovementObj);

  const TSharedPtr<FJsonObject> *DataObj;
  if (SyncItem->TryGetObjectField(TEXT("data"), DataObj)) {
    Item.Data = ParseSyncItemData(*DataObj);
  }

  return Item;
}

FRedwoodSyncItemState URedwoodCommonGameSubsystem::ParseSyncItemState(
  TSharedPtr<FJsonObject> SyncItemState
) {
  FRedwoodSyncItemState State;

  if (SyncItemState.IsValid()) {
    State.Id = SyncItemState->GetStringField(TEXT("id"));
    State.TypeId = SyncItemState->GetStringField(TEXT("typeId"));
    State.bDestroyed = SyncItemState->GetBoolField(TEXT("destroyed"));
    State.ZoneName = SyncItemState->GetStringField(TEXT("zoneName"));
  }

  return State;
}

FRedwoodSyncItemMovement URedwoodCommonGameSubsystem::ParseSyncItemMovement(
  TSharedPtr<FJsonObject> SyncItemMovement
) {
  FRedwoodSyncItemMovement Movement;

  if (SyncItemMovement.IsValid()) {
    TSharedPtr<FJsonObject> TransformObj =
      SyncItemMovement->GetObjectField(TEXT("transform"));
    if (TransformObj.IsValid()) {
      FVector Location;
      FRotator Rotation;
      FVector Scale;

      TSharedPtr<FJsonObject> LocationObj =
        TransformObj->GetObjectField(TEXT("location"));
      if (LocationObj.IsValid()) {
        Location.X = LocationObj->GetNumberField(TEXT("x"));
        Location.Y = LocationObj->GetNumberField(TEXT("y"));
        Location.Z = LocationObj->GetNumberField(TEXT("z"));
      }

      TSharedPtr<FJsonObject> RotationObj =
        TransformObj->GetObjectField(TEXT("rotation"));
      if (RotationObj.IsValid()) {
        float Roll = RotationObj->GetNumberField(TEXT("x"));
        float Pitch = RotationObj->GetNumberField(TEXT("y"));
        float Yaw = RotationObj->GetNumberField(TEXT("z"));
        Rotation = FRotator(Pitch, Yaw, Roll);
      }

      TSharedPtr<FJsonObject> ScaleObj =
        TransformObj->GetObjectField(TEXT("scale"));
      if (ScaleObj.IsValid()) {
        Scale.X = ScaleObj->GetNumberField(TEXT("x"));
        Scale.Y = ScaleObj->GetNumberField(TEXT("y"));
        Scale.Z = ScaleObj->GetNumberField(TEXT("z"));
      }

      Movement.Transform = FTransform(Rotation, Location, Scale);
    }
  }

  return Movement;
}

USIOJsonObject *URedwoodCommonGameSubsystem::ParseSyncItemData(
  TSharedPtr<FJsonObject> SyncItemData
) {
  USIOJsonObject *Data = nullptr;

  if (SyncItemData.IsValid()) {
    Data = NewObject<USIOJsonObject>();
    Data->SetRootObject(SyncItemData);
  }

  return Data;
}

bool URedwoodCommonGameSubsystem::DeserializeBackendData(
  UObject *TargetObject,
  USIOJsonObject *SIOJsonObject,
  FString VariableName,
  int32 LatestSchemaVersion
) {
  bool bDirty = false;

  if (SIOJsonObject) {
    FProperty *Prop =
      TargetObject->GetClass()->FindPropertyByName(*VariableName);
    if (Prop) {
      FStructProperty *StructProp = CastField<FStructProperty>(Prop);
      if (StructProp) {
        TSharedPtr<FJsonObject> JsonObject = SIOJsonObject->GetRootObject();

        UStruct *StructDefinition = StructProp->Struct;

        int32 SchemaVersion = -1;
        if (!JsonObject->TryGetNumberField(
              TEXT("schemaVersion"), SchemaVersion
            )) {
          // Empty objects don't need an error because it means they
          // just haven't persisted to the database yet (i.e. character
          // was created by the client but the metadata/etc hasn't been
          // synced by the game server). Nonempty objects however imply
          // they've tried to save something but it's missing schemaVersion
          if (JsonObject->Values.Num() > 0) {
            FString JsonString;
            TSharedRef<TJsonWriter<>> Writer =
              TJsonWriterFactory<>::Create(&JsonString);
            FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
            UE_LOG(
              LogRedwood,
              Error,
              TEXT(
                "schemaVersion not found in Redwood backend field for %s, did you add one to your struct? Not updating %s. %s"
              ),
              *VariableName,
              *VariableName,
              *JsonString
            );

            return false;
          }
        }

        void *StructPtr = FMemory::Malloc(StructDefinition->GetStructureSize());
        StructDefinition->InitializeStruct(StructPtr);

        if (SchemaVersion == -1) {
          bDirty = true;
          SchemaVersion = LatestSchemaVersion;

          FString MigrationFunctionName =
            FString::Printf(TEXT("%s_Creation"), *VariableName);
          UFunction *MigrationFunction =
            TargetObject->GetClass()->FindFunctionByName(*MigrationFunctionName
            );

          if (MigrationFunction) {
            // Ensure the function is valid and has the correct signature
            if (!MigrationFunction->IsValidLowLevel() || MigrationFunction->NumParms != 0) {
              UE_LOG(
                LogRedwood,
                Error,
                TEXT(
                  "Migration function %s in %s has an invalid signature, not calling."
                ),
                *MigrationFunctionName,
                *TargetObject->GetName()
              );
            } else {
              // Allocate memory for the parameters
              void *Params = FMemory::Malloc(MigrationFunction->ParmsSize);
              FMemory::Memzero(Params, MigrationFunction->ParmsSize);

              // Call the function
              TargetObject->ProcessEvent(MigrationFunction, Params);

              // Clean up
              FMemory::Free(Params);
            }
          }
        } else {
          bool bSuccess = StructDefinition == nullptr
            ? false
            : USIOJConvert::JsonObjectToUStruct(
                JsonObject, StructDefinition, StructPtr, true
              );

          if (bSuccess) {
            while (SchemaVersion < LatestSchemaVersion) {
              // call migration functions
              FString MigrationFunctionName = FString::Printf(
                TEXT("%s_Migrate_v%d"), *VariableName, SchemaVersion
              );
              UFunction *MigrationFunction =
                TargetObject->GetClass()->FindFunctionByName(
                  *MigrationFunctionName
                );

              if (MigrationFunction) {
                // Ensure the function is valid and has the correct signature
                if (!MigrationFunction->IsValidLowLevel() || MigrationFunction->NumParms != 3 || !MigrationFunction->ReturnValueOffset)
                  {
                  UE_LOG(
                    LogRedwood,
                    Error,
                    TEXT(
                      "Migration function %s in %s has an invalid signature, skipping update."
                    ),
                    *MigrationFunctionName,
                    *TargetObject->GetName()
                  );

                  break;
                } else {
                  // Allocate memory for the parameters
                  void *Params = FMemory::Malloc(MigrationFunction->ParmsSize);
                  FMemory::Memzero(Params, MigrationFunction->ParmsSize);

                  FProperty *FunctionStructProp =
                    MigrationFunction->PropertyLink;
                  FProperty *FunctionObjectProp =
                    FunctionStructProp->PropertyLinkNext;

                  // Set the input parameters
                  void *StructParam =
                    FunctionStructProp->ContainerPtrToValuePtr<void *>(Params);
                  FMemory::Memcpy(
                    StructParam, StructPtr, StructDefinition->GetStructureSize()
                  );

                  // ObjectParam is a pointer to a USIOJsonObject pointer
                  USIOJsonObject **ObjectParam =
                    FunctionObjectProp
                      ->ContainerPtrToValuePtr<USIOJsonObject *>(Params);
                  *ObjectParam = SIOJsonObject;

                  // Call the function
                  TargetObject->ProcessEvent(MigrationFunction, Params);

                  // Retrieve the return value
                  void *ReturnValue =
                    (void
                       *)((SIZE_T)Params + MigrationFunction->ReturnValueOffset);

                  // Copy the return value
                  FMemory::Free(StructPtr);
                  StructPtr =
                    FMemory::Malloc(StructDefinition->GetStructureSize());
                  FMemory::Memcpy(
                    StructPtr, ReturnValue, StructDefinition->GetStructureSize()
                  );

                  // Clean up
                  FMemory::Free(Params);
                }
              }

              SchemaVersion++;
            }

            if (SchemaVersion == LatestSchemaVersion) {
              StructProp->CopySingleValue(
                StructProp->ContainerPtrToValuePtr<void>(TargetObject),
                StructPtr
              );
            }

            FMemory::Free(StructPtr);
          } else {
            UE_LOG(
              LogRedwood,
              Error,
              TEXT("Failed to convert JSON object for %s, schemaVersion %d"),
              *TargetObject->GetName(),
              SchemaVersion
            );
          }
        }
      } else {
        UE_LOG(
          LogRedwood,
          Error,
          TEXT("%s variable in %s is not a struct"),
          *VariableName,
          *TargetObject->GetName()
        );
      }
    } else {
      bool bIsActor = TargetObject->IsA<AActor>();
      bool bIsActorComponent = TargetObject->IsA<UActorComponent>();

      if (bIsActor) {
        AActor *Actor = Cast<AActor>(TargetObject);
        UE_LOG(
          LogRedwood,
          Error,
          TEXT("%s variable not found in %s (Owner: %s)"),
          *VariableName,
          *TargetObject->GetName(),
          Actor->GetOwner() ? *Actor->GetOwner()->GetName() : TEXT("")
        );
      } else if (bIsActorComponent) {
        UActorComponent *ActorComponent = Cast<UActorComponent>(TargetObject);
        UE_LOG(
          LogRedwood,
          Error,
          TEXT("%s variable not found in %s (Owner: %s)"),
          *VariableName,
          *TargetObject->GetName(),
          ActorComponent->GetOwner() ? *ActorComponent->GetOwner()->GetName()
                                     : TEXT("")
        );
      } else {
        UE_LOG(
          LogRedwood,
          Error,
          TEXT("%s variable not found in %s"),
          *VariableName,
          *TargetObject->GetName()
        );
      }
    }
  }

  return bDirty;
}

USIOJsonObject *URedwoodCommonGameSubsystem::SerializeBackendData(
  UObject *TargetObject, FString VariableName
) {
  FProperty *Prop = TargetObject->GetClass()->FindPropertyByName(*VariableName);
  if (Prop) {
    FStructProperty *StructProp = CastField<FStructProperty>(Prop);
    if (StructProp) {
      UStruct *StructDefinition = StructProp->Struct;

      void *StructPtr = StructProp->ContainerPtrToValuePtr<void>(TargetObject);

      TSharedPtr<FJsonObject> JsonObject =
        USIOJConvert::ToJsonObject(StructDefinition, StructPtr, true);

      USIOJsonObject *SIOJsonObject = NewObject<USIOJsonObject>();
      SIOJsonObject->SetRootObject(JsonObject);

      return SIOJsonObject;
    } else {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT("%s variable in %s is not a struct"),
        *VariableName,
        *TargetObject->GetName()
      );
    }
  } else {
    bool bIsActor = TargetObject->IsA<AActor>();
    bool bIsActorComponent = TargetObject->IsA<UActorComponent>();

    if (bIsActor) {
      AActor *Actor = Cast<AActor>(TargetObject);
      UE_LOG(
        LogRedwood,
        Error,
        TEXT("%s variable not found in %s (Owner: %s)"),
        *VariableName,
        *TargetObject->GetName(),
        Actor->GetOwner() ? *Actor->GetOwner()->GetName() : TEXT("")
      );
    } else if (bIsActorComponent) {
      UActorComponent *ActorComponent = Cast<UActorComponent>(TargetObject);
      UE_LOG(
        LogRedwood,
        Error,
        TEXT("%s variable not found in %s (Owner: %s)"),
        *VariableName,
        *TargetObject->GetName(),
        ActorComponent->GetOwner() ? *ActorComponent->GetOwner()->GetName()
                                   : TEXT("")
      );
    } else {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT("%s variable not found in %s"),
        *VariableName,
        *TargetObject->GetName()
      );
    }
  }

  return nullptr;
}

bool URedwoodCommonGameSubsystem::ShouldUseBackend(UWorld *World) {
  if (!World) {
    return false;
  }

#if WITH_EDITOR
  URedwoodEditorSettings *RedwoodEditorSettings =
    GetMutableDefault<URedwoodEditorSettings>();

  return World->WorldType != EWorldType::PIE ||
    RedwoodEditorSettings->bUseBackendInPIE;
#else
  return true;
#endif
}

ERedwoodFriendListType URedwoodCommonGameSubsystem::ParseFriendListType(
  FString StringValue
) {
  ERedwoodFriendListType EnumValue = ERedwoodFriendListType::Unknown;

  if (StringValue == TEXT("active")) {
    EnumValue = ERedwoodFriendListType::Active;
  } else if (StringValue == TEXT("pending-all")) {
    EnumValue = ERedwoodFriendListType::PendingAll;
  } else if (StringValue == TEXT("pending-received")) {
    EnumValue = ERedwoodFriendListType::PendingReceived;
  } else if (StringValue == TEXT("pending-sent")) {
    EnumValue = ERedwoodFriendListType::PendingSent;
  } else if (StringValue == TEXT("blocked")) {
    EnumValue = ERedwoodFriendListType::Blocked;
  }

  return EnumValue;
}

ERedwoodGuildInviteType URedwoodCommonGameSubsystem::ParseGuildInviteType(
  FString StringValue
) {
  if (StringValue == TEXT("public")) {
    return ERedwoodGuildInviteType::Public;
  } else if (StringValue == TEXT("admin")) {
    return ERedwoodGuildInviteType::Admin;
  } else if (StringValue == TEXT("member")) {
    return ERedwoodGuildInviteType::Member;
  }

  return ERedwoodGuildInviteType::Unknown;
}

FString URedwoodCommonGameSubsystem::SerializeGuildInviteType(
  ERedwoodGuildInviteType InviteType
) {
  switch (InviteType) {
    case ERedwoodGuildInviteType::Public:
      return TEXT("public");
    case ERedwoodGuildInviteType::Admin:
      return TEXT("admin");
    case ERedwoodGuildInviteType::Member:
      return TEXT("member");
    default:
      return TEXT("unknown");
  }
}

ERedwoodGuildAndAllianceMemberState
URedwoodCommonGameSubsystem::ParseGuildAndAllianceMemberState(
  FString StringValue
) {
  if (StringValue == TEXT("none")) {
    return ERedwoodGuildAndAllianceMemberState::None;
  } else if (StringValue == TEXT("invited")) {
    return ERedwoodGuildAndAllianceMemberState::Invited;
  } else if (StringValue == TEXT("member")) {
    return ERedwoodGuildAndAllianceMemberState::Member;
  } else if (StringValue == TEXT("banned")) {
    return ERedwoodGuildAndAllianceMemberState::Banned;
  } else if (StringValue == TEXT("admin")) {
    return ERedwoodGuildAndAllianceMemberState::Admin;
  }

  return ERedwoodGuildAndAllianceMemberState::Unknown;
}

FString URedwoodCommonGameSubsystem::SerializeGuildAndAllianceMemberState(
  ERedwoodGuildAndAllianceMemberState State
) {
  switch (State) {
    case ERedwoodGuildAndAllianceMemberState::None:
      return TEXT("none");
    case ERedwoodGuildAndAllianceMemberState::Invited:
      return TEXT("invited");
    case ERedwoodGuildAndAllianceMemberState::Member:
      return TEXT("member");
    case ERedwoodGuildAndAllianceMemberState::Banned:
      return TEXT("banned");
    case ERedwoodGuildAndAllianceMemberState::Admin:
      return TEXT("admin");
    default:
      return TEXT("unknown");
  }
}

FRedwoodGuild URedwoodCommonGameSubsystem::ParseGuild(
  TSharedPtr<FJsonObject> GuildObject
) {
  FRedwoodGuild Guild;

  Guild.Id = GuildObject->GetStringField(TEXT("id"));
  FDateTime::ParseIso8601(
    *GuildObject->GetStringField(TEXT("createdAt")), Guild.CreatedAt
  );
  FDateTime::ParseIso8601(
    *GuildObject->GetStringField(TEXT("updatedAt")), Guild.UpdatedAt
  );
  Guild.Name = GuildObject->GetStringField(TEXT("name"));
  Guild.Tag = GuildObject->GetStringField(TEXT("tag"));
  Guild.InviteType = URedwoodCommonGameSubsystem::ParseGuildInviteType(
    GuildObject->GetStringField(TEXT("inviteType"))
  );
  Guild.bListed = GuildObject->GetBoolField(TEXT("listed"));
  Guild.bMembershipPublic = GuildObject->GetBoolField(TEXT("membershipPublic"));

  return Guild;
}

FRedwoodGuildInfo URedwoodCommonGameSubsystem::ParseGuildInfo(
  TSharedPtr<FJsonObject> GuildInfoObject
) {
  FRedwoodGuildInfo GuildInfo;

  TSharedPtr<FJsonObject> GuildObject =
    GuildInfoObject->GetObjectField(TEXT("guild"));
  GuildInfo.Guild = URedwoodCommonGameSubsystem::ParseGuild(GuildObject);

  GuildInfo.PlayerState =
    URedwoodCommonGameSubsystem::ParseGuildAndAllianceMemberState(
      GuildInfoObject->GetStringField(TEXT("playerState"))
    );

  const TArray<TSharedPtr<FJsonValue>> *AlliancesArray;
  if (GuildInfoObject->TryGetArrayField(TEXT("alliances"), AlliancesArray)) {
    for (const TSharedPtr<FJsonValue> &AllianceValue : *AlliancesArray) {
      TSharedPtr<FJsonObject> AllianceObject = AllianceValue->AsObject();
      FRedwoodGuildAllianceMembership AllianceMembership;

      AllianceMembership.AllianceId =
        AllianceObject->GetStringField(TEXT("allianceId"));
      AllianceMembership.AllianceName =
        AllianceObject->GetStringField(TEXT("allianceName"));
      AllianceMembership.GuildState =
        URedwoodCommonGameSubsystem::ParseGuildAndAllianceMemberState(
          AllianceObject->GetStringField(TEXT("guildState"))
        );

      GuildInfo.Alliances.Add(AllianceMembership);
    }
  }

  return GuildInfo;
}

FRedwoodAlliance URedwoodCommonGameSubsystem::ParseAlliance(
  TSharedPtr<FJsonObject> AllianceObj
) {
  FRedwoodAlliance OutAlliance;

  OutAlliance.Id = AllianceObj->GetStringField(TEXT("id"));
  FDateTime::ParseIso8601(
    *AllianceObj->GetStringField(TEXT("createdAt")), OutAlliance.CreatedAt
  );
  FDateTime::ParseIso8601(
    *AllianceObj->GetStringField(TEXT("updatedAt")), OutAlliance.UpdatedAt
  );
  OutAlliance.Name = AllianceObj->GetStringField(TEXT("name"));
  OutAlliance.bInviteOnly = AllianceObj->GetBoolField(TEXT("inviteOnly"));

  return OutAlliance;
}
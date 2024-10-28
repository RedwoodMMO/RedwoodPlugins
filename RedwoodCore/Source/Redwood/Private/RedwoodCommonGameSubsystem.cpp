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
  if (ZoneData->TryGetArrayField(TEXT("persistentItems"), PersistentItems)) {
    for (const TSharedPtr<FJsonValue> &Item : *PersistentItems) {
      if (Item->Type == EJson::Object) {
        TSharedPtr<FJsonObject> ItemObj = Item->AsObject();
        FRedwoodPersistentItem PersistentItem = ParsePersistentItem(ItemObj);
        Data.PersistentItems.Add(PersistentItem);
      }
    }
  }

  return Data;
}

FRedwoodPersistentItem URedwoodCommonGameSubsystem::ParsePersistentItem(
  TSharedPtr<FJsonObject> PersistentItem
) {
  FRedwoodPersistentItem Item;

  Item.Id = PersistentItem->GetStringField(TEXT("id"));

  Item.TypeId = PersistentItem->GetStringField(TEXT("typeId"));

  TSharedPtr<FJsonObject> TransformObj =
    PersistentItem->GetObjectField(TEXT("transform"));
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

    Item.Transform = FTransform(Rotation, Location, Scale);
  }

  const TSharedPtr<FJsonObject> *DataObj;
  if (PersistentItem->TryGetObjectField(TEXT("data"), DataObj)) {
    Item.Data = NewObject<USIOJsonObject>();
    Item.Data->SetRootObject(*DataObj);
  }

  return Item;
}

void URedwoodCommonGameSubsystem::DeserializeBackendData(
  UObject *TargetObject,
  USIOJsonObject *SIOJsonObject,
  FString VariableName,
  int32 LatestSchemaVersion
) {
  if (SIOJsonObject) {
    FProperty *Prop =
      TargetObject->GetClass()->FindPropertyByName(*VariableName);
    if (Prop) {
      FStructProperty *StructProp = CastField<FStructProperty>(Prop);
      if (StructProp) {
        TSharedPtr<FJsonObject> JsonObject = SIOJsonObject->GetRootObject();

        UStruct *StructDefinition = StructProp->Struct;

        int32 SchemaVersion = 0;
        if (!JsonObject->TryGetNumberField(
              TEXT("schemaVersion"), SchemaVersion
            )) {
          // Empty objects don't need an error because it means they
          // just haven't persisted to the database yet (i.e. character
          // was created by the client but the metadata/etc hasn't been
          // synced by the game server). Nonempty objects however imply
          // they've tried to save something but it's missing schemaVersion
          if (JsonObject->Values.Num() > 0) {
            UE_LOG(
              LogRedwood,
              Error,
              TEXT(
                "schemaVersion not found in Redwood backend field for %s, did you add one to your struct? Not updating %s."
              ),
              *VariableName,
              *VariableName
            );
          }

          return;
        }

        void *StructPtr = FMemory::Malloc(StructDefinition->GetStructureSize());
        FMemory::Memzero(StructPtr, StructDefinition->GetStructureSize());

        bool bSuccess = StructDefinition == nullptr
          ? false
          : USIOJConvert::JsonObjectToUStruct(
              JsonObject, StructDefinition, StructPtr, true
            );

        if (bSuccess) {
          while (SchemaVersion < LatestSchemaVersion) {
            // call a migration functions
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

                FProperty *FunctionStructProp = MigrationFunction->PropertyLink;
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
                  FunctionObjectProp->ContainerPtrToValuePtr<USIOJsonObject *>(
                    Params
                  );
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
              StructProp->ContainerPtrToValuePtr<void>(TargetObject), StructPtr
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
    UE_LOG(
      LogRedwood,
      Error,
      TEXT("%s variable not found in %s"),
      *VariableName,
      *TargetObject->GetName()
    );
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

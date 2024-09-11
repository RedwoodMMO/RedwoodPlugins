// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodCommonGameSubsystem.h"

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

  FString SavePath = FPaths::ProjectSavedDir() / TEXT("Characters");
  FPaths::NormalizeFilename(SavePath);
  FString FileName = SavePath / (Character.Id + TEXT(".json"));

  FFileHelper::SaveStringToFile(OutputString, *FileName);
}

TArray<FRedwoodCharacterBackend>
URedwoodCommonGameSubsystem::LoadAllCharactersFromDisk() {
  FString SavePath = FPaths::ProjectSavedDir() / TEXT("Characters");
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
  FString SavePath = FPaths::ProjectSavedDir() / TEXT("Characters");
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
  FString SavePath = FPaths::ProjectSavedDir() / TEXT("Characters");
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

  Server.MapId = ServerProxy->GetStringField(TEXT("mapId"));

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
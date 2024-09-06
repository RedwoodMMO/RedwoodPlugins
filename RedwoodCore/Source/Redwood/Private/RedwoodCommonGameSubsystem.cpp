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

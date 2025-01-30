// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodModule.h"
#include "Types/RedwoodTypes.h"

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "RedwoodCommonGameSubsystem.generated.h"

class USIOJsonObject;

UCLASS(BlueprintType)
class REDWOOD_API URedwoodCommonGameSubsystem : public UGameInstanceSubsystem {
  GENERATED_BODY()

public:
  // Begin USubsystem
  virtual void Initialize(FSubsystemCollectionBase &Collection) override;
  virtual void Deinitialize() override;
  // End USubsystem

  static void SaveCharacterToDisk(FRedwoodCharacterBackend &Character);

  static TArray<FRedwoodCharacterBackend> LoadAllCharactersFromDisk();

  static FRedwoodCharacterBackend LoadCharacterFromDisk(FString CharacterId);

  static uint8 GetCharactersOnDiskCount();

  static FRedwoodCharacterBackend ParseCharacter(
    TSharedPtr<FJsonObject> CharacterObj
  );
  static FRedwoodGameServerProxy ParseServerProxy(
    TSharedPtr<FJsonObject> ServerProxy
  );
  static FRedwoodGameServerInstance ParseServerInstance(
    TSharedPtr<FJsonObject> ServerInstance
  );
  static FRedwoodZoneData ParseZoneData(TSharedPtr<FJsonObject> ZoneData);

  static FRedwoodSyncItem ParseSyncItem(TSharedPtr<FJsonObject> SyncItem);
  static FRedwoodSyncItemState ParseSyncItemState(
    TSharedPtr<FJsonObject> SyncItemState
  );
  static FRedwoodSyncItemMovement ParseSyncItemMovement(
    TSharedPtr<FJsonObject> SyncItemMovement
  );
  static USIOJsonObject *ParseSyncItemData(TSharedPtr<FJsonObject> SyncItemData
  );

  static USIOJsonObject *SerializeBackendData(
    UObject *TargetObject, FString VariableName
  );

  static void DeserializeBackendData(
    UObject *TargetObject,
    USIOJsonObject *SIOJsonObject,
    FString VariableName,
    int32 LatestSchemaVersion
  );

  UFUNCTION(BlueprintPure, Category = "Redwood")
  static bool ShouldUseBackend(UWorld *World);

private:
};

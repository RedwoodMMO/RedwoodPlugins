// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodModule.h"
#include "Types/RedwoodTypes.h"

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "RedwoodCommonGameSubsystem.generated.h"

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

private:
};

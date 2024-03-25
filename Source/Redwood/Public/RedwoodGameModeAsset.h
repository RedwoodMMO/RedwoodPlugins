// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "RedwoodGameMode.h"
#include "RedwoodGameModeBase.h"

#include "RedwoodGameModeAsset.generated.h"

UENUM(BlueprintType)
enum class ERedwoodGameMode { GameModeBase, GameMode };

UCLASS(BlueprintType)
class URedwoodGameModeAsset : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Redwood)
  FName RedwoodId;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = GameMode)
  FName GameModeName;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = GameMode)
  ERedwoodGameMode GameModeType;

  UPROPERTY(
    BlueprintReadWrite,
    EditAnywhere,
    Category = GameMode,
    meta =
      (EditCondition = "GameModeType == ERedwoodGameMode::GameModeBase",
       EditConditionHides)
  )
  TSubclassOf<ARedwoodGameModeBase> GameModeBaseClass;

  UPROPERTY(
    BlueprintReadWrite,
    EditAnywhere,
    Category = GameMode,
    meta =
      (EditCondition = "GameModeType == ERedwoodGameMode::GameMode",
       EditConditionHides)
  )
  TSubclassOf<ARedwoodGameMode> GameModeClass;
};

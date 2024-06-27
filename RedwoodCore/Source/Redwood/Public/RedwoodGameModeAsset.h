// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "RedwoodGameMode.h"
#include "RedwoodGameModeBase.h"

#include "RedwoodGameModeAsset.generated.h"

UENUM(BlueprintType)
enum class ERedwoodGameModeType : uint8 { GameModeBase, GameMode };

UCLASS(BlueprintType)
class URedwoodGameModeAsset : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Redwood)
  FString RedwoodId;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = GameMode)
  FText GameModeName;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = GameMode)
  ERedwoodGameModeType GameModeType;

  UPROPERTY(
    BlueprintReadWrite,
    EditAnywhere,
    Category = GameMode,
    meta =
      (EditCondition = "GameModeType == ERedwoodGameModeType::GameModeBase",
       EditConditionHides)
  )
  TSubclassOf<ARedwoodGameModeBase> GameModeBaseClass;

  UPROPERTY(
    BlueprintReadWrite,
    EditAnywhere,
    Category = GameMode,
    meta =
      (EditCondition = "GameModeType == ERedwoodGameModeType::GameMode",
       EditConditionHides)
  )
  TSubclassOf<ARedwoodGameMode> GameModeClass;
};

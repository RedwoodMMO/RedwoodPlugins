// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "RedwoodSyncItemAsset.generated.h"

UCLASS(BlueprintType)
class URedwoodSyncItemAsset : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  // Use "proxy" for the asset that refers to GameServerProxy
  // data (aka world data)
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Redwood)
  FString RedwoodTypeId;

  // This class can be empty for the asset used for world data
  // because the current GameState will be referenced instead
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Redwood)
  TSoftClassPtr<AActor> ActorClass;
};

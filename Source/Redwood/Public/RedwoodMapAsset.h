// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "RedwoodMapAsset.generated.h"

UCLASS(BlueprintType)
class URedwoodMapAsset : public UPrimaryDataAsset {
  GENERATED_BODY()

public:
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Redwood)
  FString RedwoodId;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Map)
  FText MapName;

  UPROPERTY(
    BlueprintReadWrite,
    EditAnywhere,
    Category = Map,
    meta = (AllowedTypes = "Map")
  )
  FPrimaryAssetId MapId;
};

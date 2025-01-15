// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodTypesCommon.h"

#include "RedwoodTypesSync.generated.h"

USTRUCT(BlueprintType)
struct FRedwoodSyncItemState {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString TypeId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bDestroyed = false;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString ZoneName;
};

USTRUCT(BlueprintType)
struct FRedwoodSyncItemMovement {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FTransform Transform;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FTransform RatePerSecond;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *AnimationState = nullptr;
};

USTRUCT(BlueprintType)
struct FRedwoodSyncItem {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FRedwoodSyncItemState State;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FRedwoodSyncItemMovement Movement;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data = nullptr;
};

USTRUCT(BlueprintType)
struct FRedwoodZoneData {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data = nullptr;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRedwoodSyncItem> Items;
};

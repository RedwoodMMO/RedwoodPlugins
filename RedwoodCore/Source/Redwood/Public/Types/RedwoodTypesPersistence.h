// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodTypesCommon.h"

#include "RedwoodTypesPersistence.generated.h"

USTRUCT(BlueprintType)
struct FRedwoodPersistentItem {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString TypeId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FTransform Transform;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data = nullptr;
};

USTRUCT(BlueprintType)
struct FRedwoodZoneData {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data = nullptr;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRedwoodPersistentItem> PersistentItems;
};

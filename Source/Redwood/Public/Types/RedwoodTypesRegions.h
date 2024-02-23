// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodTypesCommon.h"

#include "RedwoodTypesRegions.generated.h"

USTRUCT(BlueprintType)
struct FRedwoodRegion {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Name;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Ping;
};

USTRUCT(BlueprintType)
struct FRedwoodRegionsChanged {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRedwoodRegion> Regions;
};

USTRUCT(BlueprintType)
struct FRedwoodRegionLatency {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Url;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<float> RTTs;
};

USTRUCT(BlueprintType)
struct FRedwoodRegionLatencySort {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  float RTT = 0.0f;
};

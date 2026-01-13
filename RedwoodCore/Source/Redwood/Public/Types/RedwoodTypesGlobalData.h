// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodTypesCommon.h"

#include "RedwoodTypesGlobalData.generated.h"

USTRUCT(BlueprintType)
struct FRedwoodGetGlobalDataOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  int32 Id = -1;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data = nullptr;
};

typedef TDelegate<void(const FRedwoodGetGlobalDataOutput &)>
  FRedwoodGetGlobalDataOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodGetGlobalDataOutputDynamicDelegate, FRedwoodGetGlobalDataOutput, Data
);

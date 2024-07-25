// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodTypesCommon.h"

#include "RedwoodTypesAuth.generated.h"

UENUM(BlueprintType)
enum class ERedwoodAuthUpdateType : uint8 {
  Success,
  MustVerifyAccount,
  Error,
  Unknown
};

USTRUCT(BlueprintType)
struct FRedwoodAuthUpdate {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  ERedwoodAuthUpdateType Type = ERedwoodAuthUpdateType::Unknown;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Message;
};

typedef TDelegate<void(const FRedwoodAuthUpdate &)> FRedwoodAuthUpdateDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodAuthUpdateDynamicDelegate, FRedwoodAuthUpdate, Data
);

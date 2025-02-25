// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodTypesCommon.h"

#include "RedwoodTypesAuth.generated.h"

USTRUCT(BlueprintType)
struct FRedwoodPlayer {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString PlayerId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Nickname;
};

USTRUCT(BlueprintType)
struct FRedwoodListPlayersOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRedwoodPlayer> Players;
};

typedef TDelegate<void(const FRedwoodListPlayersOutput &)>
  FRedwoodListPlayersOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodListPlayersOutputDynamicDelegate, FRedwoodListPlayersOutput, Data
);

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

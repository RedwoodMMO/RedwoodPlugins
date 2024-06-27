// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodTypesCommon.h"

#include "RedwoodTypesTicketing.generated.h"

UENUM(BlueprintType)
enum class ERedwoodTicketingUpdateType : uint8 {
  JoinResponse,
  Update,
  TicketError,
  Unknown
};

USTRUCT(BlueprintType)
struct FRedwoodTicketingUpdate {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  ERedwoodTicketingUpdateType Type = ERedwoodTicketingUpdateType::Unknown;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Message;
};

typedef TDelegate<void(const FRedwoodTicketingUpdate &)>
  FRedwoodTicketingUpdateDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodTicketingUpdateDynamicDelegate, FRedwoodTicketingUpdate, Update
);

// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SIOJsonObject.h"

#include "RedwoodTypesAuth.h"
#include "RedwoodTypesBlobs.h"
#include "RedwoodTypesCharacters.h"
#include "RedwoodTypesGlobalData.h"
#include "RedwoodTypesGuilds.h"
#include "RedwoodTypesParties.h"
#include "RedwoodTypesPlayers.h"
#include "RedwoodTypesPlayersGuilds.h"
#include "RedwoodTypesRealms.h"
#include "RedwoodTypesRegions.h"
#include "RedwoodTypesServers.h"
#include "RedwoodTypesSync.h"
#include "RedwoodTypesTicketing.h"

#include "RedwoodTypes.generated.h"

USTRUCT(BlueprintType)
struct FRedwoodSocketConnected {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;
};

typedef TDelegate<void(const FRedwoodSocketConnected &)>
  FRedwoodSocketConnectedDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodSocketConnectedDynamicDelegate, FRedwoodSocketConnected, Details
);

typedef TDelegate<void(const FString &)> FRedwoodErrorOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodErrorOutputDynamicDelegate, FString, Error
);

typedef TDelegate<void()> FRedwoodDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRedwoodDynamicDelegate);

#define RW_ENUM_TO_STRING(EnumValue) \
  UEnum::GetValueAsString(EnumValue).RightChop( \
    UEnum::GetValueAsString(EnumValue).Find("::") + 2 \
  )

#define RW_STRING_TO_ENUM(EnumType, StringValue) \
  (EnumType) UEnum::GetValueByString(#EnumType, StringValue)
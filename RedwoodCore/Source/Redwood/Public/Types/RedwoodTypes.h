// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SIOJsonObject.h"

#include "RedwoodTypesAuth.h"
#include "RedwoodTypesBlobs.h"
#include "RedwoodTypesCharacters.h"
#include "RedwoodTypesPersistence.h"
#include "RedwoodTypesRealms.h"
#include "RedwoodTypesRegions.h"
#include "RedwoodTypesServers.h"
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
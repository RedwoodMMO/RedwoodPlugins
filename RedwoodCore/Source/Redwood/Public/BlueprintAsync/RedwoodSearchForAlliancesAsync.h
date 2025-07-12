// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodSearchForAlliancesAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodSearchForAlliancesAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Search for Alliances",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodSearchForAlliancesAsync *SearchForAlliances(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString SearchText,
    bool bIncludePartialMatches
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodListAlliancesOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString SearchText;
  bool bIncludePartialMatches;
};
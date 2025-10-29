// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodListAlliancesAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodListAlliancesAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  // @param GuildIdFilter If empty, all alliances will be listed.
  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "List Alliances",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodListAlliancesAsync *ListAlliances(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString GuildIdFilter
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodListAlliancesOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString GuildIdFilter;
};
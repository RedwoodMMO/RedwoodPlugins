// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodSetPartyDataAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodSetPartyDataAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Respond to Party Invite",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodSetPartyDataAsync *SetPartyData(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString LootType,
    USIOJsonObject *PartyData = nullptr
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodGetPartyOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString LootType;

  UPROPERTY()
  USIOJsonObject *PartyData = nullptr;
};
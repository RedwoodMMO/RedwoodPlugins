// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodGetOrCreatePartyAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodGetOrCreatePartyAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Get or Create Party",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodGetOrCreatePartyAsync *GetOrCreateParty(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    bool bCreateIfNotInParty = false
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodGetPartyOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  bool bCreateIfNotInParty = false;
};
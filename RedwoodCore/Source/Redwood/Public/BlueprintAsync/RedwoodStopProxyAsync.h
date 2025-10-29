// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodStopProxyAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodStopProxyAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Stop Proxy",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodStopProxyAsync *StopProxy(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString ServerProxyId
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString ServerProxyId;
};
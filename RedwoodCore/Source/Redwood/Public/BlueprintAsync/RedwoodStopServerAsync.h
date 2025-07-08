// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodStopServerAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodStopServerAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Stop Server",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodStopServerAsync *StopServer(
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
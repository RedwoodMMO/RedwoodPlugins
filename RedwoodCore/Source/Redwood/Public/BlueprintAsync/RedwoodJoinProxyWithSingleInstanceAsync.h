// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodJoinProxyWithSingleInstanceAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodJoinProxyWithSingleInstanceAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Join Proxy with Single Instance",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodJoinProxyWithSingleInstanceAsync *JoinProxyWithSingleInstance(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString ProxyReference,
    FString Password
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodJoinServerOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString ProxyReference;
  FString Password;
};
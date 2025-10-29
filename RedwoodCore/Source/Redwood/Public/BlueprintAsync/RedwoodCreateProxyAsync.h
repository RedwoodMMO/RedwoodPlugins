// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodCreateProxyAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodCreateProxyAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Create Proxy",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodCreateProxyAsync *CreateProxy(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    bool bJoinSession,
    FRedwoodCreateProxyInput Parameters
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodCreateProxyOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  bool bJoinSession;

  UPROPERTY()
  FRedwoodCreateProxyInput Parameters;
};
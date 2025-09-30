// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodListProxiesAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodListProxiesAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "List Proxies",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodListProxiesAsync *ListProxies(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    TArray<FString> PrivateProxyReferences
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodListProxiesOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  TArray<FString> PrivateProxyReferences;
};
// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodListServersAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodListServersAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "List Servers",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodListServersAsync *ListServers(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    TArray<FString> PrivateServerReferences
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodListServersOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  TArray<FString> PrivateServerReferences;
};
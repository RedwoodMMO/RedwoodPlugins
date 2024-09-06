// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodListRealmsAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodListRealmsAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "List Realms",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodListRealmsAsync *ListRealms(
    URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodListRealmsOutputDynamicDelegate OnOutput;

  URedwoodClientGameSubsystem *Target;
};
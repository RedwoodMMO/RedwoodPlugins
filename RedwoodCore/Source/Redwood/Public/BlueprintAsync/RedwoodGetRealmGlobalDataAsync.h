// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodServerGameSubsystem.h"

#include "RedwoodGetRealmGlobalDataAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodGetRealmGlobalDataAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Get Realm Global Data",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodGetRealmGlobalDataAsync *GetRealmGlobalData(
    URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodGetGlobalDataOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;
};
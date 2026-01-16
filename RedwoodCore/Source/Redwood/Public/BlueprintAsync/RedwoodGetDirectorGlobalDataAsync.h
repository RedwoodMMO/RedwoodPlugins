// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodServerGameSubsystem.h"

#include "RedwoodGetDirectorGlobalDataAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodGetDirectorGlobalDataAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Get Director Global Data",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodGetDirectorGlobalDataAsync *GetDirectorGlobalData(
    URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodGetGlobalDataOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;
};
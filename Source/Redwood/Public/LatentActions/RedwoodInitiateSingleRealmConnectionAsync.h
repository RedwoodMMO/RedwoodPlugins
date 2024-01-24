// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodLatentCommon.h"
#include "RedwoodTitleGameSubsystem.h"

#include "RedwoodInitiateSingleRealmConnectionAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodInitiateSingleRealmConnectionAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Initiate Single-Realm Connection",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodInitiateSingleRealmConnectionAsync *
  InitializeSingleRealmConnection(
    URedwoodTitleGameSubsystem *Target, UObject *WorldContextObject
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodOnSocketConnectedLatent OnResult;

  URedwoodTitleGameSubsystem *Target;
};
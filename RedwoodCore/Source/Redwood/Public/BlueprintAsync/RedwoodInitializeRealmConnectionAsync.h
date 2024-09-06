// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodInitializeRealmConnectionAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodInitializeRealmConnectionAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Initialize Realm Connection",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodInitializeRealmConnectionAsync *InitializeRealmConnection(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FRedwoodRealm Realm
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodSocketConnectedDynamicDelegate OnOutput;

  URedwoodClientGameSubsystem *Target;

  FRedwoodRealm Realm;
};
// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodTitleGameSubsystem.h"

#include "RedwoodInitiateRealmConnectionAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodInitiateRealmConnectionAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Initiate Realm Connection",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodInitiateRealmConnectionAsync *InitializeRealmConnection(
    URedwoodTitleGameSubsystem *Target,
    UObject *WorldContextObject,
    FRedwoodRealm Realm
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodOnSocketConnectedAsync OnResult;

  URedwoodTitleGameSubsystem *Target;

  FRedwoodRealm Realm;
};
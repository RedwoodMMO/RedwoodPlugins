// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodInitializeConnectionForFirstRealmAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodInitializeConnectionForFirstRealmAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  // This is a helper function for scenarios that only have a single realm
  // (i.e. games that only have one realm or during dev/testing environments).
  // It calls ListRealms and then InitializeRealmConnection for the first realm returned.
  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Initialize Connection For First Realm",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodInitializeConnectionForFirstRealmAsync *
  InitializeConnectionForFirstRealm(
    URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodSocketConnectedDynamicDelegate OnOutput;

  URedwoodClientGameSubsystem *Target;
};
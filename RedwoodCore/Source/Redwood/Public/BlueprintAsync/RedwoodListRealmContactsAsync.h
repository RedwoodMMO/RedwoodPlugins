// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodListRealmContactsAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodListRealmContactsAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "List Realm Contacts",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodListRealmContactsAsync *ListRealmContacts(
    URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodListRealmContactsOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;
};
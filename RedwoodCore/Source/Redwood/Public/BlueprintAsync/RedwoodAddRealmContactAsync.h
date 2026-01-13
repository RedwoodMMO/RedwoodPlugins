// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodAddRealmContactAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodAddRealmContactAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Add Realm Contact",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodAddRealmContactAsync *AddRealmContact(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString OtherCharacterId,
    bool bBlocked
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString OtherCharacterId;
  bool bBlocked;
};
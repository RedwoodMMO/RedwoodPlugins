// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodRemoveRealmContactAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodRemoveRealmContactAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Remove Realm Contact",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodRemoveRealmContactAsync *RemoveRealmContact(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString OtherCharacterId
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString OtherCharacterId;
};
// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodLatentCommon.h"
#include "RedwoodTitleGameSubsystem.h"

#include "RedwoodListRealmsAsync.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodListRealmsResultLatent, FRedwoodRealmsResult, Result
);

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
    URedwoodTitleGameSubsystem *Target, UObject *WorldContextObject
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodListRealmsResultLatent OnResult;

  URedwoodTitleGameSubsystem *Target;
};
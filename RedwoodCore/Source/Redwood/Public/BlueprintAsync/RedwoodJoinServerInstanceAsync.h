// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodTitleGameSubsystem.h"

#include "RedwoodJoinServerInstanceAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodJoinServerInstanceAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Join Server Instance",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodJoinServerInstanceAsync *JoinServerInstance(
    URedwoodTitleGameSubsystem *Target,
    UObject *WorldContextObject,
    FString ServerReference,
    FString Password
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodJoinServerOutputDynamicDelegate OnOutput;

  URedwoodTitleGameSubsystem *Target;

  FString ServerReference;
  FString Password;
};
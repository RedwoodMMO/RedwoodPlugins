// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodPromoteToPartyLeaderAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodPromoteToPartyLeaderAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Promote to Party Leader",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodPromoteToPartyLeaderAsync *PromoteToPartyLeader(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString TargetPlayerId
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString TargetPlayerId;
};
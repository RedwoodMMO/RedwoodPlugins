// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodInviteToPartyAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodInviteToPartyAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Invite to Party",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodInviteToPartyAsync *InviteToParty(
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
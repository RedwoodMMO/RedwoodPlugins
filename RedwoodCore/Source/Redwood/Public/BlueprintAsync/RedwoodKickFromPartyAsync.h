// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodKickFromPartyAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodKickFromPartyAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Kick from Party",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodKickFromPartyAsync *KickFromParty(
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
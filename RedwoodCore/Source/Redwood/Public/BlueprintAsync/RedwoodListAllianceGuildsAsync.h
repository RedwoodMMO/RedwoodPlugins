// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodListAllianceGuildsAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodListAllianceGuildsAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "List Alliance Guilds",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodListAllianceGuildsAsync *ListAllianceGuilds(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString AllianceId,
    ERedwoodGuildAndAllianceMemberState State
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodListAllianceGuildsOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString AllianceId;
  ERedwoodGuildAndAllianceMemberState State =
    ERedwoodGuildAndAllianceMemberState::Unknown;
};
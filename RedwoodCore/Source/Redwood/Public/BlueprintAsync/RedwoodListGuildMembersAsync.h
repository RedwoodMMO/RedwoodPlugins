// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodListGuildMembersAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodListGuildMembersAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "List Guild Members",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodListGuildMembersAsync *ListGuildMembers(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString GuildId,
    ERedwoodGuildAndAllianceMemberState State
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodListGuildMembersOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString GuildId;
  ERedwoodGuildAndAllianceMemberState State =
    ERedwoodGuildAndAllianceMemberState::Unknown;
};

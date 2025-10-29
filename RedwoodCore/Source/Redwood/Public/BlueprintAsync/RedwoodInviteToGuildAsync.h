// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodInviteToGuildAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodInviteToGuildAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Invite To Guild",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodInviteToGuildAsync *InviteToGuild(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString GuildId,
    FString TargetPlayerId
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString GuildId;
  FString TargetPlayerId;
};
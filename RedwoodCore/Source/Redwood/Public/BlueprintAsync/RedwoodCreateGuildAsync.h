// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodCreateGuildAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodCreateGuildAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Create Guild",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodCreateGuildAsync *CreateGuild(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString GuildName,
    FString GuildTag,
    ERedwoodGuildInviteType InviteType,
    bool bListed,
    bool bMembershipPublic
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodCreateGuildOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString GuildName;
  FString GuildTag;
  ERedwoodGuildInviteType InviteType = ERedwoodGuildInviteType::Unknown;
  bool bListed = false;
  bool bMembershipPublic = false;
};
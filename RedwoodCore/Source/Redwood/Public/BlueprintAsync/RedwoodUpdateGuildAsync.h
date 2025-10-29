// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodUpdateGuildAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodUpdateGuildAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Update Guild",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodUpdateGuildAsync *UpdateGuild(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString GuildId,
    FString GuildName,
    FString GuildTag,
    ERedwoodGuildInviteType InviteType,
    bool bListed,
    bool bMembershipPublic
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString GuildId;
  FString GuildName;
  FString GuildTag;
  ERedwoodGuildInviteType InviteType = ERedwoodGuildInviteType::Unknown;
  bool bListed;
  bool bMembershipPublic;
};
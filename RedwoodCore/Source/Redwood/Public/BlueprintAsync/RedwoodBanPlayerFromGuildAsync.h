// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodBanPlayerFromGuildAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodBanPlayerFromGuildAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Ban Player From Guild",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodBanPlayerFromGuildAsync *BanPlayerFromGuild(
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
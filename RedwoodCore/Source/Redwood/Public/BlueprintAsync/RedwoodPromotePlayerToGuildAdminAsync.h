// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodPromotePlayerToGuildAdminAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodPromotePlayerToGuildAdminAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Promote Player To Guild Admin",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodPromotePlayerToGuildAdminAsync *PromotePlayerToGuildAdmin(
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
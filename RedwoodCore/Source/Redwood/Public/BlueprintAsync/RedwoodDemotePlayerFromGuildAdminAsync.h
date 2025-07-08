// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodDemotePlayerFromGuildAdminAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodDemotePlayerFromGuildAdminAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Demote Player From Guild Admin",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodDemotePlayerFromGuildAdminAsync *DemotePlayerFromGuildAdmin(
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
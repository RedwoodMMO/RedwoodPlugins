// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodUnbanGuildFromAllianceAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodUnbanGuildFromAllianceAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Unban Guild From Alliance",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodUnbanGuildFromAllianceAsync *UnbanGuildFromAlliance(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString GuildId,
    FString AllianceId
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString GuildId;
  FString AllianceId;
};
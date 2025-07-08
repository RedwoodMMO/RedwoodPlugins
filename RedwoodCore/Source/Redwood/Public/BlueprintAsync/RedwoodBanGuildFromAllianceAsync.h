// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodBanGuildFromAllianceAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodBanGuildFromAllianceAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Ban Guild From Alliance",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodBanGuildFromAllianceAsync *BanGuildFromAlliance(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString AllianceId,
    FString GuildId
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString AllianceId;
  FString GuildId;
};
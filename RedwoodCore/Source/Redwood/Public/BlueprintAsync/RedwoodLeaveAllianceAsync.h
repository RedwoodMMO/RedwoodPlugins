// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodLeaveAllianceAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodLeaveAllianceAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Leave Alliance",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodLeaveAllianceAsync *LeaveAlliance(
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
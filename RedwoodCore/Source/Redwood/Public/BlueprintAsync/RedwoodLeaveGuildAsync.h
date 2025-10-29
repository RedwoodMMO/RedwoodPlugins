// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodLeaveGuildAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodLeaveGuildAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Leave Guild",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodLeaveGuildAsync *LeaveGuild(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString GuildId
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString GuildId;
};
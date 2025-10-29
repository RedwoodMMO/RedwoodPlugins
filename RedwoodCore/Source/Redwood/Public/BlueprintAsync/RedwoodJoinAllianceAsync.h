// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodJoinAllianceAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodJoinAllianceAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Join Alliance",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodJoinAllianceAsync *JoinAlliance(
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
// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodUpdateAllianceAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodUpdateAllianceAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Update Alliance",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodUpdateAllianceAsync *UpdateAlliance(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString AllianceId,
    FString AllianceName,
    bool bInviteOnly
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString AllianceId;
  FString AllianceName;
  bool bInviteOnly;
};
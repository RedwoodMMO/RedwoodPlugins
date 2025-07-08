// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodCreateAllianceAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodCreateAllianceAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Create Alliance",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodCreateAllianceAsync *CreateAlliance(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString AllianceName,
    FString GuildId,
    bool bInviteOnly
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodCreateAllianceOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString AllianceName;
  FString GuildId;
  bool bInviteOnly = false;
};
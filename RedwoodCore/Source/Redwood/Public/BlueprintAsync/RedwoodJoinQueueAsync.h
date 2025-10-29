// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodClientGameSubsystem.h"

#include "RedwoodJoinQueueAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodJoinQueueAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Join Queue",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodJoinQueueAsync *JoinQueue(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString ProxyId,
    FString ZoneName,
    bool bTransferWholeParty
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodTicketingUpdateDynamicDelegate OnUpdate;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString ProxyId;
  FString ZoneName;
  bool bTransferWholeParty;
};
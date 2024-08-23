// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodTitleGameSubsystem.h"

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
    URedwoodTitleGameSubsystem *Target,
    UObject *WorldContextObject,
    FString ProxyId,
    FString ZoneName
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodTicketingUpdateDynamicDelegate OnUpdate;

  URedwoodTitleGameSubsystem *Target;

  FString ProxyId;
  FString ZoneName;
};
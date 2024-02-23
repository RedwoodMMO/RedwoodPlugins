// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodTitleGameSubsystem.h"

#include "RedwoodJoinTicketingAsync.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodTicketingUpdateAsync, FRedwoodTicketingUpdate, Update
);

UCLASS()
class REDWOOD_API URedwoodJoinTicketingAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Join Ticketing",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodJoinTicketingAsync *JoinTicketing(
    URedwoodTitleGameSubsystem *Target,
    UObject *WorldContextObject,
    FString Profile
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodTicketingUpdateAsync OnUpdate;

  URedwoodTitleGameSubsystem *Target;

  FString Profile;

  UPROPERTY()
  USIOJsonObject *Data;
};
// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodTitleGameSubsystem.h"

#include "RedwoodJoinTicketingAsync.generated.h"

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
    TArray<FString> ModeIds,
    TArray<FString> Regions
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodTicketingUpdateDynamicDelegate OnUpdate;

  URedwoodTitleGameSubsystem *Target;

  TArray<FString> ModeIds;
  TArray<FString> Regions;

  UPROPERTY()
  USIOJsonObject *Data;
};
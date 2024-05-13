// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodTitleGameSubsystem.h"

#include "RedwoodJoinMatchmakingAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodJoinMatchmakingAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Join Matchmaking",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodJoinMatchmakingAsync *JoinMatchmaking(
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
};
// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodClientGameSubsystem.h"

#include "RedwoodJoinCustomAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodJoinCustomAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Join Custom",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodJoinCustomAsync *JoinCustom(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    bool bTransferWholeParty,
    TArray<FString> Regions
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodTicketingUpdateDynamicDelegate OnUpdate;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  bool bTransferWholeParty;
  TArray<FString> Regions;
};
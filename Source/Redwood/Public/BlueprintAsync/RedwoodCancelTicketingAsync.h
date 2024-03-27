// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodTitleGameSubsystem.h"

#include "RedwoodCancelTicketingAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodCancelTicketingAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Cancel Ticketing",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodCancelTicketingAsync *CancelTicketing(
    URedwoodTitleGameSubsystem *Target, UObject *WorldContextObject
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  URedwoodTitleGameSubsystem *Target;
};
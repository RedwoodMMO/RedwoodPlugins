// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodTitleGameSubsystem.h"

#include "RedwoodLeaveTicketingAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodLeaveTicketingAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Leave Ticketing",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodLeaveTicketingAsync *LeaveTicketing(
    URedwoodTitleGameSubsystem *Target, UObject *WorldContextObject
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  URedwoodTitleGameSubsystem *Target;
};
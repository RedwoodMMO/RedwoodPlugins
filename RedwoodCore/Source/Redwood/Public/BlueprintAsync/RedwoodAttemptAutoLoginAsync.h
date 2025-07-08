// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodAttemptAutoLoginAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodAttemptAutoLoginAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Attempt Auto-Login",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodAttemptAutoLoginAsync *AttemptAutoLogin(
    URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodAuthUpdateDynamicDelegate OnUpdate;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;
};
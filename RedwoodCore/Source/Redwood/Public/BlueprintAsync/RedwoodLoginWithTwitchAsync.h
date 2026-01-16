// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodLoginWithTwitchAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodLoginWithTwitchAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Login with Twitch",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodLoginWithTwitchAsync *LoginWithTwitch(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    bool bRememberMe
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodAuthUpdateDynamicDelegate OnUpdate;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  bool bRememberMe;
};
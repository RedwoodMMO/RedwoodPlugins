// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodLoginWithDiscordAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodLoginWithDiscordAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Login with Discord",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodLoginWithDiscordAsync *LoginWithDiscord(
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
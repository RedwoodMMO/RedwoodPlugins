// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodLoginAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodLoginAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Login",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodLoginAsync *Login(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    const FString &Username,
    const FString &Password,
    bool bRememberMe
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodAuthUpdateDynamicDelegate OnUpdate;

  URedwoodClientGameSubsystem *Target;

  FString Username;

  FString Password;

  bool bRememberMe;
};
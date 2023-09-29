// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodAuthCommon.h"

#include "RedwoodRegisterUsernameAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodRegisterUsernameAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodRegisterUsernameAsync *RegisterUsername(
    UObject *WorldContextObject,
    ARedwoodTitlePlayerController *PlayerController,
    const FString &Username,
    const FString &Password
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodAuthResponse Updated;

  ARedwoodTitlePlayerController *PlayerController;

  FString Username;

  FString Password;

  void HandleUpdated(ERedwoodAuthUpdateType Type, FString Message);
};
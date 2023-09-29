// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodAuthCommon.h"

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
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodLoginAsync *Login(
    UObject *WorldContextObject,
    ARedwoodTitlePlayerController *PlayerController,
    const FString &EmailOrUsername,
    const FString &PasswordOrToken
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodAuthResponse Updated;

  ARedwoodTitlePlayerController *PlayerController;

  FString EmailOrUsername;

  FString PasswordOrToken;

  void HandleUpdated(ERedwoodAuthUpdateType Type, FString Message);
};
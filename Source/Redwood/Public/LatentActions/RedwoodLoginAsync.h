// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodLatentCommon.h"

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
       DisplayName = "Login (Latent)",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodLoginAsync *Login(
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
// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodLatentCommon.h"

#include "RedwoodRegisterAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodRegisterAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Register (Latent)",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodRegisterAsync *Register(
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

  UFUNCTION()
  void HandleUpdated(ERedwoodAuthUpdateType Type, FString Message);
};
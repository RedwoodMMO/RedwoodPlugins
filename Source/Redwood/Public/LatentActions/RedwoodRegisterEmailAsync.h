// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodAuthCommon.h"

#include "RedwoodRegisterEmailAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodRegisterEmailAsync
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
  static URedwoodRegisterEmailAsync *RegisterEmail(
    UObject *WorldContextObject,
    ARedwoodTitlePlayerController *PlayerController,
    const FString &Email,
    const FString &Password,
    const FString &DisplayName
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodAuthResponse Updated;

  ARedwoodTitlePlayerController *PlayerController;

  FString Email;

  FString Password;

  FString DisplayName;

  void HandleUpdated(ERedwoodAuthUpdateType Type, FString Message);
};
// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodTitleGameSubsystem.h"

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
       DisplayName = "Register",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodRegisterAsync *Register(
    URedwoodTitleGameSubsystem *Target,
    UObject *WorldContextObject,
    const FString &Username,
    const FString &Password
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodAuthUpdateAsync OnUpdate;

  URedwoodTitleGameSubsystem *Target;

  FString Username;

  FString Password;
};
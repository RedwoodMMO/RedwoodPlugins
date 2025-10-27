// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodClientGameSubsystem.h"
#include "Types/RedwoodTypes.h"

#include "RedwoodLoginWithEOSAsync.generated.h"

UCLASS()
class REDWOODEOS_API URedwoodLoginWithEOSAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Login with EOS",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodLoginWithEOSAsync *LoginWithEOS(
    URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodAuthUpdateDynamicDelegate OnUpdate;

  URedwoodClientGameSubsystem *Target;
};
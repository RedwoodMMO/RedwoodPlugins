// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodClientGameSubsystem.h"
#include "Types/RedwoodTypes.h"

#include "RedwoodLoginWithSteamAsync.generated.h"

UCLASS()
class REDWOODSTEAM_API URedwoodLoginWithSteamAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Login with Steam",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodLoginWithSteamAsync *LoginWithSteam(
    URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodAuthUpdateDynamicDelegate OnUpdate;

  URedwoodClientGameSubsystem *Target;
};
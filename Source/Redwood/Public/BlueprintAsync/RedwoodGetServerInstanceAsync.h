// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodTitleGameSubsystem.h"

#include "RedwoodGetServerInstanceAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodGetServerInstanceAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Get Server Instance",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodGetServerInstanceAsync *GetServerInstance(
    URedwoodTitleGameSubsystem *Target,
    UObject *WorldContextObject,
    FString ServerReference,
    FString Password
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodGetServerResultAsync OnResult;

  URedwoodTitleGameSubsystem *Target;

  FString ServerReference;
  FString Password;
};
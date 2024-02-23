// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodLatentCommon.h"
#include "RedwoodTitleGameSubsystem.h"

#include "RedwoodCreateServerAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodCreateServerAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Create Server",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodCreateServerAsync *CreateServer(
    URedwoodTitleGameSubsystem *Target,
    UObject *WorldContextObject,
    FRedwoodCreateServerParameters Parameters
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodGetServerResultLatent OnResult;

  URedwoodTitleGameSubsystem *Target;

  FRedwoodCreateServerParameters Parameters;
};
// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

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
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    bool bJoinSession,
    FRedwoodCreateServerInput Parameters
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodCreateServerOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  bool bJoinSession;

  FRedwoodCreateServerInput Parameters;
};
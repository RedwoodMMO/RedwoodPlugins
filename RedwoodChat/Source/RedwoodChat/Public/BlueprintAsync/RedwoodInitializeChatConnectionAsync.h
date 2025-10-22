// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "BlueprintAsync/RedwoodAsyncCommon.h"
#include "RedwoodChatClientSubsystem.h"

#include "RedwoodInitializeChatConnectionAsync.generated.h"

UCLASS()
class REDWOODCHAT_API URedwoodInitializeChatConnectionAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Initialize Chat Connection",
       Category = "Redwood Chat",
       WorldContext = "WorldContextObject")
  )
  static URedwoodInitializeChatConnectionAsync *InitializeChatConnection(
    URedwoodClientChatSubsystem *Target, UObject *WorldContextObject
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientChatSubsystem *Target;
};
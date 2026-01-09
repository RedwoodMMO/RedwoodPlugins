// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "BlueprintAsync/RedwoodAsyncCommon.h"
#include "RedwoodChatClientSubsystem.h"

#include "RedwoodCreateCustomRoomAsync.generated.h"

UCLASS()
class REDWOODCHAT_API URedwoodCreateCustomRoomAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Create Custom Room",
       Category = "Redwood Chat",
       WorldContext = "WorldContextObject")
  )
  static URedwoodCreateCustomRoomAsync *CreateCustomRoom(
    URedwoodClientChatSubsystem *Target,
    UObject *WorldContextObject,
    FString Id,
    FString Password
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientChatSubsystem *Target;

  FString Id;
  FString Password;
};
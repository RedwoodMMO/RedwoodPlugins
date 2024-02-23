// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodTitleGameSubsystem.h"

#include "RedwoodCreateCharacterAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodCreateCharacterAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Create Character",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodCreateCharacterAsync *CreateCharacter(
    URedwoodTitleGameSubsystem *Target,
    UObject *WorldContextObject,
    USIOJsonObject *Data
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodCharacterResultAsync OnResult;

  URedwoodTitleGameSubsystem *Target;

  UPROPERTY() USIOJsonObject *Data;
};
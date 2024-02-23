// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodTitleGameSubsystem.h"

#include "RedwoodListCharactersAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodListCharactersAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "List Characters",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodListCharactersAsync *ListCharacters(
    URedwoodTitleGameSubsystem *Target, UObject *WorldContextObject
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodListCharactersOutputDynamicDelegate OnOutput;

  URedwoodTitleGameSubsystem *Target;
};
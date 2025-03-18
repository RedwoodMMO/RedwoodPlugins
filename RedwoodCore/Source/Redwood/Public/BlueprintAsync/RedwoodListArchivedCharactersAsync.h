// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodListArchivedCharactersAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodListArchivedCharactersAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "List Archived Characters",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodListArchivedCharactersAsync *ListArchivedCharacters(
    URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodListCharactersOutputDynamicDelegate OnOutput;

  URedwoodClientGameSubsystem *Target;
};
// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodGetCharacterDataAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodGetCharacterDataAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Get Character Data",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodGetCharacterDataAsync *GetCharacterData(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString CharacterId
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodGetCharacterOutputDynamicDelegate OnOutput;

  URedwoodClientGameSubsystem *Target;

  FString CharacterId;
};
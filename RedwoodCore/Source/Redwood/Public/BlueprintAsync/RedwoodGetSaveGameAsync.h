// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodServerGameSubsystem.h"

#include "RedwoodGetSaveGameAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodGetSaveGameAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Get SaveGame",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodGetSaveGameAsync *GetSaveGame(
    URedwoodServerGameSubsystem *Target,
    UObject *WorldContextObject,
    FString Key
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodGetSaveGameOutputDynamicDelegate OnOutput;

  URedwoodServerGameSubsystem *Target;

  FString Key;
};
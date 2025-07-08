// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodServerGameSubsystem.h"

#include "RedwoodPutSaveGameAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodPutSaveGameAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Put SaveGame",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodPutSaveGameAsync *PutSaveGame(
    URedwoodServerGameSubsystem *Target,
    UObject *WorldContextObject,
    FString Key,
    USaveGame *SaveGame
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodServerGameSubsystem *Target;

  FString Key;

  UPROPERTY()
  USaveGame *SaveGame;
};
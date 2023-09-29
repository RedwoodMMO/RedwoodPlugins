// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodTitlePlayerController.h"

#include "RedwoodSetCharacterDataAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodSetCharacterDataAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodSetCharacterDataAsync *SetCharacterData(
    UObject *WorldContextObject,
    ARedwoodTitlePlayerController *PlayerController,
    USIOJsonObject *Data
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodCharacterResponse OnResponse;

  ARedwoodTitlePlayerController *PlayerController;

  UPROPERTY()
  USIOJsonObject *Data;

  void HandleResponse(FString Error, USIOJsonObject *CharacterData);
};
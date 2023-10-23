// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodLatentCommon.h"
#include "RedwoodTitlePlayerController.h"

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
       DisplayName = "Create Character (Latent)",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodCreateCharacterAsync *CreateCharacter(
    UObject *WorldContextObject,
    ARedwoodTitlePlayerController *PlayerController,
    USIOJsonObject *Data
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodCharacterResponseLatent OnResponse;

  ARedwoodTitlePlayerController *PlayerController;

  UPROPERTY()
  USIOJsonObject *Data;

  void HandleResponse(FString Error, FRedwoodPlayerCharacter Character);
};
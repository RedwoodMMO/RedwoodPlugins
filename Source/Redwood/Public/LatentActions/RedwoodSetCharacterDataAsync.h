// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodLatentCommon.h"
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
       DisplayName = "Set Character Data (Latent)",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodSetCharacterDataAsync *SetCharacterData(
    UObject *WorldContextObject,
    ARedwoodTitlePlayerController *PlayerController,
    FString CharacterId,
    USIOJsonObject *Data
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodCharacterResponseLatent OnResponse;

  ARedwoodTitlePlayerController *PlayerController;

  FString CharacterId;

  UPROPERTY()
  USIOJsonObject *Data;

  UFUNCTION()
  void HandleResponse(FString Error, FRedwoodPlayerCharacter Character);
};
// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodTitlePlayerController.h"

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
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodGetCharacterDataAsync *GetCharacterData(
    UObject *WorldContextObject,
    ARedwoodTitlePlayerController *PlayerController,
    FString CharacterId
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodCharacterResponse OnResponse;

  ARedwoodTitlePlayerController *PlayerController;

  FString CharacterId;

  void HandleResponse(FString Error, FRedwoodPlayerCharacter Character);
};
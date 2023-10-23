// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodLatentCommon.h"
#include "RedwoodTitlePlayerController.h"

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
       DisplayName = "List Characters (Latent)",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodListCharactersAsync *ListCharacters(
    UObject *WorldContextObject, ARedwoodTitlePlayerController *PlayerController
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodCharactersResponseLatent OnResponse;

  ARedwoodTitlePlayerController *PlayerController;

  void HandleResponse(
    FString Error, TArray<FRedwoodPlayerCharacter> Characters
  );
};
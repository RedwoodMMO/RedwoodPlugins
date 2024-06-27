// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "RedwoodSaveGame.generated.h"

UCLASS(BlueprintType)
class REDWOOD_API URedwoodSaveGame : public USaveGame {
  GENERATED_BODY()

public:
  UPROPERTY(VisibleAnywhere, Category = Authentication)
  FString Username;

  UPROPERTY(VisibleAnywhere, Category = Authentication)
  FString AuthToken;
};

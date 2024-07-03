// Copyright Incanta LLC. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "RedwoodPlayerState.generated.h"

class USIOJsonObject;

UCLASS(BlueprintType, Blueprintable)
class REDWOOD_API ARedwoodPlayerState : public APlayerState {
  GENERATED_BODY()

public:
  UPROPERTY(BlueprintReadOnly, Category = "Redwood|PlayerState")
  FString RedwoodPlayerId;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood|PlayerState")
  FString CharacterId;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood|PlayerState")
  USIOJsonObject *CharacterMetadata;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood|PlayerState")
  USIOJsonObject *CharacterEquippedInventory;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood|PlayerState")
  USIOJsonObject *CharacterNonequippedInventory;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood|PlayerState")
  USIOJsonObject *CharacterData;
};

// Copyright Incanta LLC. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "Types/RedwoodTypesCharacters.h"

#include "RedwoodPlayerState.generated.h"

class USIOJsonObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRedwoodCharacterUpdated);

UCLASS(BlueprintType, Blueprintable)
class REDWOOD_API ARedwoodPlayerState : public APlayerState {
  GENERATED_BODY()

public:
  UPROPERTY(BlueprintReadWrite, Category = "Redwood|PlayerState")
  FRedwoodCharacterBackend RedwoodCharacter;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood|PlayerState")
  bool bClientReady = false;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood|PlayerState")
  bool bServerReady = false;

  UFUNCTION(
    BlueprintCallable,
    Server,
    Reliable,
    WithValidation,
    Category = "Redwood|PlayerState"
  )
  void SetClientReady();

  UFUNCTION(BlueprintCallable, Category = "Redwood|PlayerState")
  void SetServerReady();

  UPROPERTY(BlueprintAssignable, Category = "Events")
  FOnRedwoodCharacterUpdated OnRedwoodCharacterUpdated;
};

// Copyright Incanta LLC. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "Types/RedwoodTypesCharacters.h"

#include "RedwoodPlayerState.generated.h"

class USIOJsonObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRedwoodPlayerStateUpdated);

UCLASS(BlueprintType, Blueprintable)
class REDWOOD_API ARedwoodPlayerState : public APlayerState {
  GENERATED_BODY()

public:
  // NOT AVAILABLE ON CLIENTS
  FRedwoodPlayerData RedwoodPlayer;
  // NOT AVAILABLE ON CLIENTS
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

  UFUNCTION(BlueprintCallable, Category = "Redwood|PlayerState")
  void SetRedwoodPlayer(FRedwoodPlayerData InRedwoodPlayer);

  UFUNCTION(BlueprintCallable, Category = "Redwood|PlayerState")
  void SetRedwoodCharacter(FRedwoodCharacterBackend InRedwoodCharacter);

  UPROPERTY(BlueprintAssignable, Category = "Events")
  FOnRedwoodPlayerStateUpdated OnRedwoodCharacterUpdated;

  UPROPERTY(BlueprintAssignable, Category = "Events")
  FOnRedwoodPlayerStateUpdated OnRedwoodPlayerUpdated;
};

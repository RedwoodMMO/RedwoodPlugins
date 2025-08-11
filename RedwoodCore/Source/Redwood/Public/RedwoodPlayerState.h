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
  ARedwoodPlayerState(
    const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get()
  );

  // NOT AVAILABLE ON CLIENTS
  FRedwoodPlayerData RedwoodPlayer;
  // NOT AVAILABLE ON CLIENTS
  FRedwoodCharacterBackend RedwoodCharacter;

  //~ Begin AActor interface
  virtual void Tick(float DeltaSeconds) override;
  //~ End AActor interface

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redwood")
  bool bFollowPawn = false;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  bool bClientReady = false;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  bool bServerReady = false;

  UFUNCTION(
    BlueprintCallable,
    Server,
    Reliable,
    WithValidation,
    Category = "Redwood|PlayerState"
  )
  void SetClientReady();

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void SetServerReady();

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void SetRedwoodPlayer(FRedwoodPlayerData InRedwoodPlayer);

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void SetRedwoodCharacter(FRedwoodCharacterBackend InRedwoodCharacter);

  UPROPERTY(BlueprintAssignable, Category = "Events")
  FOnRedwoodPlayerStateUpdated OnRedwoodCharacterUpdated;

  UPROPERTY(BlueprintAssignable, Category = "Events")
  FOnRedwoodPlayerStateUpdated OnRedwoodPlayerUpdated;
};

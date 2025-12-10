// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RedwoodGameModeComponent.h"

#include "RedwoodGameModeBase.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Redwood))
class REDWOOD_API ARedwoodGameModeBase : public AGameModeBase {
  GENERATED_BODY()

public:
  ARedwoodGameModeBase(
    const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get()
  );

  //~AGameModeBase interface
  virtual void InitGame(
    const FString &MapName, const FString &Options, FString &ErrorMessage
  ) override;

  virtual APlayerController *Login(
    UPlayer *NewPlayer,
    ENetRole InRemoteRole,
    const FString &Portal,
    const FString &Options,
    const FUniqueNetIdRepl &UniqueId,
    FString &ErrorMessage
  ) override;

  virtual bool PlayerCanRestart_Implementation(APlayerController *Player
  ) override;

  virtual void FinishRestartPlayer(
    AController *NewPlayer, const FRotator &StartRotation
  ) override;

  virtual APawn *SpawnDefaultPawnAtTransform_Implementation(
    AController *NewPlayer, const FTransform &SpawnTransform
  );
  //~End of AGameModeBase interface

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  float DatabasePersistenceInterval = 0.5f;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  float PostBeginPlayDelay = 0.2f;

private:
  UPROPERTY()
  URedwoodGameModeComponent *GameModeComponent;
};

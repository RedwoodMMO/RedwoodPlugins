// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "RedwoodGameModeComponent.h"

#include "RedwoodGameMode.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Redwood))
class REDWOOD_API ARedwoodGameMode : public AGameMode {
  GENERATED_UCLASS_BODY()

public:
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

private:
  UPROPERTY()
  URedwoodGameModeComponent *GameModeComponent;
};

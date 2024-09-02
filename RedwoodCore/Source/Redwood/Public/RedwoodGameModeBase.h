// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "SocketIONative.h"

#include "RedwoodGameModeBase.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Redwood))
class REDWOOD_API ARedwoodGameModeBase : public AGameModeBase {
  GENERATED_BODY()

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

  UFUNCTION(BlueprintCallable, Category = "Redwood|GameMode")
  TArray<FString> GetExpectedCharacterIds() const;

  UFUNCTION(BlueprintCallable, Category = "Redwood|GameMode")
  void OnGameModeLogout(AGameModeBase *GameMode, AController *Controller);

private:
  TSharedPtr<FSocketIONative> Sidecar;
};

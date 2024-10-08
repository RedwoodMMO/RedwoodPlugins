// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "SocketIONative.h"

#include "RedwoodGameMode.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Redwood))
class REDWOOD_API ARedwoodGameMode : public AGameMode {
  GENERATED_BODY()

public:
  //~AActor interface
  virtual void BeginPlay() override;
  //~End of AActor interface

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

  UFUNCTION()
  void PostBeginPlay();

  UFUNCTION(BlueprintCallable, Category = "Redwood|GameMode")
  TArray<FString> GetExpectedCharacterIds() const;

  UFUNCTION(BlueprintCallable, Category = "Redwood|GameMode")
  void OnGameModeLogout(AGameModeBase *GameMode, AController *Controller);

  UFUNCTION()
  void FlushPersistence();

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  float DatabasePersistenceInterval = 0.5f;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  float PostBeginPlayDelay = 0.2f;

private:
  TSharedPtr<FSocketIONative> Sidecar;

  FTimerHandle FlushPlayerCharacterDataTimerHandle;
  FTimerHandle PostBeginPlayTimerHandle;
};

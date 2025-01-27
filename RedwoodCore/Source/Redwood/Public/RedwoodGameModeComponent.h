// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "SocketIONative.h"

#include "RedwoodGameModeComponent.generated.h"

UCLASS()
class REDWOOD_API URedwoodGameModeComponent : public UActorComponent {
  GENERATED_BODY()

public:
  //~UActorComponent interface
  virtual void BeginPlay() override;
  virtual void TickComponent(
    float DeltaTime,
    enum ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction
  ) override;
  //~End of UActorComponent interface

  //~AGameModeBase interface
  void InitGame(
    const FString &MapName, const FString &Options, FString &ErrorMessage
  );

  APlayerController *Login(
    UPlayer *NewPlayer,
    ENetRole InRemoteRole,
    const FString &Portal,
    const FString &Options,
    const FUniqueNetIdRepl &UniqueId,
    FString &ErrorMessage,
    std::function<APlayerController
                    *(UPlayer *,
                      ENetRole,
                      const FString &,
                      const FString &,
                      const FUniqueNetIdRepl &,
                      FString &)> SuperDelegate
  );

  bool PlayerCanRestart_Implementation(
    APlayerController *Player,
    std::function<bool(APlayerController *)> SuperDelegate
  );

  void FinishRestartPlayer(
    AController *NewPlayer,
    const FRotator &StartRotation,
    std::function<void(AController *)> FailedToRestartPlayerDelegate
  );

  FTransform PickPawnSpawnTransform(
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

  bool bPostBeganPlay = false;
};

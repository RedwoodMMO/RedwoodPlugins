// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "SocketIONative.h"

#include "RedwoodGameModeComponent.generated.h"

class URedwoodServerGameSubsystem;

UCLASS()
class REDWOOD_API URedwoodGameModeComponent : public UActorComponent {
  GENERATED_BODY()

public:
  URedwoodGameModeComponent(const FObjectInitializer &ObjectInitializer);

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

  void PostLogin(APlayerController *NewPlayer);

  /**
   * Server-side. Called by `URedwoodPlayerStateComponent::
   * Server_SubmitJoinToken_Implementation` when the client's Server
   * RPC delivers the realm-frontend-issued bearer token. Looks up the
   * pending {playerId, characterId, playerController} stashed during
   * Login() for this connection and runs the sidecar verification
   * with the supplied token. Kicks the connection if no pending entry
   * exists (token without preceding Login), if the sidecar rejects,
   * or if anything else is amiss.
   *
   * Token never travels in the URL options, only over the
   * Server RPC channel, so it can't leak via LogNet URL prints.
   */
  void ReceiveClientAuthToken(
    UNetConnection *Connection, const FString &Token
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

  void InitVariables(
    float InDatabasePersistenceInterval, float InPostBeginPlayDelay
  ) {
    DatabasePersistenceInterval = InDatabasePersistenceInterval;
    PostBeginPlayDelay = InPostBeginPlayDelay;
  };

private:
  /**
   * Authoritative core of player-auth verification: hits the sidecar
   * with {playerId, characterId, token} and either marks the
   * PlayerController as ready (PlayerStateComponent->SetServerReady)
   * or kicks. Called from `Login()` for the legacy URL-Token path and
   * from `ReceiveClientAuthToken()` for the Server-RPC path.
   */
  void RunSidecarPlayerAuth(
    APlayerController *PlayerController,
    const FString &PlayerId,
    const FString &CharacterId,
    const FString &Token
  );

  /**
   * Per-connection state for the deferred-auth (Server-RPC) path.
   * Created in Login() when RedwoodAuth=1 is present in URL options
   * but no Token (the new client path); consumed in
   * ReceiveClientAuthToken().
   */
  struct FPendingAuth {
    TWeakObjectPtr<APlayerController> PlayerController;
    FString PlayerId;
    FString CharacterId;
    double CreatedAtSeconds = 0.0;
  };
  TMap<TWeakObjectPtr<UNetConnection>, FPendingAuth> PendingAuthByConnection;

  /**
   * Kicks any pending-auth entry that's been waiting too long for
   * its Server_SubmitJoinToken RPC. Run on a recurring timer started
   * in BeginPlay.
   */
  void PruneStalePendingAuth();
  FTimerHandle PendingAuthPruneTimerHandle;

  float DatabasePersistenceInterval;
  float PostBeginPlayDelay;

  TSharedPtr<FSocketIONative> Sidecar;

  FTimerHandle FlushPersistentDataTimerHandle;
  FTimerHandle PostBeginPlayTimerHandle;

  bool bPostBeganPlay = false;

  UPROPERTY()
  URedwoodServerGameSubsystem *ServerSubsystem = nullptr;
};

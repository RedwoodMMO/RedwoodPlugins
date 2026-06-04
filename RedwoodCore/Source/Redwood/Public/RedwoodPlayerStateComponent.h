// Copyright Incanta LLC. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "Types/RedwoodTypes.h"

#include "RedwoodPlayerStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRedwoodPlayerStateUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRedwoodPlayerTransferring);

UCLASS(
  BlueprintType,
  Blueprintable,
  ClassGroup = (Redwood),
  meta = (BlueprintSpawnableComponent)
)
class REDWOOD_API URedwoodPlayerStateComponent : public UActorComponent {
  GENERATED_BODY()

public:
  URedwoodPlayerStateComponent(const FObjectInitializer &ObjectInitializer);

  // NOT AVAILABLE ON CLIENTS
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FRedwoodPlayerData RedwoodPlayer;
  // NOT AVAILABLE ON CLIENTS
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FRedwoodCharacterBackend RedwoodCharacter;

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkCharacterDataDirty() {
    bCharacterDataDirty = true;
  }

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  bool IsCharacterDataDirty() const {
    return bCharacterDataDirty;
  }

  //~ Begin UActorComponent Interface
  virtual void BeginPlay() override;
  virtual void TickComponent(
    float DeltaTime,
    enum ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction
  ) override;
  //~ End UActorComponent Interface

  /**
   * Server RPC the local client invokes in BeginPlay to hand the
   * realm-frontend-issued bearer token to the game server. Replaces
   * the old URL-option `?Token=…` path so the secret never
   * lands in `LogNet`-style URL prints.
   *
   * The server routes the token through
   * `URedwoodGameModeComponent::ReceiveClientAuthToken`, which looks
   * up the connection's deferred-auth entry and runs the sidecar
   * verification.
   */
  UFUNCTION(
    Server,
    Reliable,
    WithValidation,
    Category = "Redwood|PlayerState"
  )
  void Server_SubmitJoinToken(const FString &Token);

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

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  bool GetSpawnData(FTransform &Transform, FRotator &ControlRotation);

  UPROPERTY(BlueprintAssignable, Category = "Events")
  FOnRedwoodPlayerStateUpdated OnRedwoodCharacterUpdated;

  UPROPERTY(BlueprintAssignable, Category = "Events")
  FOnRedwoodPlayerStateUpdated OnRedwoodPlayerUpdated;

  bool bTransferring = false;

  /**
   * Server-only entry point that begins a zone transfer for this player.
   * Marks the component as transferring and notifies the owning client
   * via Client_OnTransferring, which broadcasts OnTransferring locally.
   * Called from URedwoodServerGameSubsystem's TravelPlayerToZone* paths
   * in place of setting bTransferring directly.
   */
  void InitTransferring();

  /**
   * Reliable RPC delivered only to this PlayerState's owning client.
   * Broadcasts OnTransferring so C++/Blueprint listeners on the owning
   * client can react (e.g. show a loading screen) before the travel.
   */
  UFUNCTION(Client, Reliable, Category = "Redwood|PlayerState")
  void Client_OnTransferring();

  // Broadcast on the owning client when a zone transfer begins. Bind in
  // C++ or Blueprints. Only fires on the owning client (see
  // Client_OnTransferring); it does not fire on the server or other
  // clients.
  UPROPERTY(BlueprintAssignable, Category = "Events")
  FOnRedwoodPlayerTransferring OnTransferring;

  void ClearDirtyFlags() {
    bCharacterDataDirty = false;
  }

  bool bRanPostLogin = false;

private:
  // Attempts to hand the realm-issued join token to the server via
  // Server_SubmitJoinToken. Safe to call repeatedly; it no-ops until the
  // owning PlayerController is linked and is the local controller, and
  // only ever sends once. Called from BeginPlay and retried from
  // TickComponent to ride out the BeginPlay-time owner-link race.
  void TrySubmitJoinToken();

  TWeakObjectPtr<APlayerState> OwnerPlayerState = nullptr;

  bool bCharacterDataDirty = false;

  // Client-side join-token submission state (see TrySubmitJoinToken).
  bool bWantsToSubmitJoinToken = false;
  bool bJoinTokenSubmitted = false;
  float JoinTokenSubmitElapsed = 0.0f;
};

// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodTitlePlayerController.h"

#include "RedwoodJoinLobbyAsync.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
  FRedwoodLobbyUpdateLatent, ERedwoodLobbyUpdateType, Type, FString, Message
);

UCLASS()
class REDWOOD_API URedwoodJoinLobbyAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Join Lobby (Latent)",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodJoinLobbyAsync *JoinLobby(
    UObject *WorldContextObject,
    ARedwoodTitlePlayerController *PlayerController,
    FString Profile
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodLobbyUpdateLatent OnUpdate;

  ARedwoodTitlePlayerController *PlayerController;

  FString Profile;

  UPROPERTY()
  USIOJsonObject *Data;

  UFUNCTION()
  void HandleUpdate(ERedwoodLobbyUpdateType Type, FString Message);
};
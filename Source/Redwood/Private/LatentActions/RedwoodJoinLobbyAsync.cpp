// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodJoinLobbyAsync.h"

URedwoodJoinLobbyAsync *URedwoodJoinLobbyAsync::JoinLobby(
  UObject *WorldContextObject,
  ARedwoodTitlePlayerController *PlayerController,
  FString Profile
) {
  URedwoodJoinLobbyAsync *Action = NewObject<URedwoodJoinLobbyAsync>();
  Action->PlayerController = PlayerController;
  Action->Profile = Profile;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodJoinLobbyAsync::Activate() {
  FRedwoodLobbyUpdate Delegate;
  Delegate.BindDynamic(this, &URedwoodJoinLobbyAsync::HandleUpdate);
  PlayerController->JoinLobby(Profile, Delegate);
}

void URedwoodJoinLobbyAsync::HandleUpdate(
  ERedwoodLobbyUpdateType Type, FString Message
) {
  OnUpdate.Broadcast(Type, Message);
}

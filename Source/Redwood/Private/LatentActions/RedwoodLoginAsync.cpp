// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodLoginAsync.h"

URedwoodLoginAsync *URedwoodLoginAsync::Login(
  UObject *WorldContextObject,
  ARedwoodTitlePlayerController *PlayerController,
  const FString &EmailOrUsername,
  const FString &PasswordOrToken
) {
  URedwoodLoginAsync *Action = NewObject<URedwoodLoginAsync>();
  Action->PlayerController = PlayerController;
  Action->EmailOrUsername = EmailOrUsername;
  Action->EmailOrUsername = EmailOrUsername;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodLoginAsync::Activate() {
  FRedwoodAuthUpdate Delegate;
  Delegate.AddDynamic(this, &URedwoodLoginAsync::HandleUpdated);
  PlayerController->Login(EmailOrUsername, PasswordOrToken, Delegate);
}

void URedwoodLoginAsync::HandleUpdated(
  ERedwoodAuthUpdateType Type, FString Message
) {
  Updated.Broadcast(Type, Message);
}

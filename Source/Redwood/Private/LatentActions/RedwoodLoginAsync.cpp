// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodLoginAsync.h"

URedwoodLoginAsync *URedwoodLoginAsync::Login(
  UObject *WorldContextObject,
  ARedwoodTitlePlayerController *PlayerController,
  const FString &Username,
  const FString &Password
) {
  URedwoodLoginAsync *Action = NewObject<URedwoodLoginAsync>();
  Action->PlayerController = PlayerController;
  Action->Username = Username;
  Action->Password = Password;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodLoginAsync::Activate() {
  FRedwoodAuthUpdate Delegate;
  Delegate.BindDynamic(this, &URedwoodLoginAsync::HandleUpdated);
  PlayerController->Login(Username, Password, Delegate);
}

void URedwoodLoginAsync::HandleUpdated(
  ERedwoodAuthUpdateType Type, FString Message
) {
  Updated.Broadcast(Type, Message);
}

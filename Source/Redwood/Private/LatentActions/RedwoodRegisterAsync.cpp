// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodRegisterAsync.h"

URedwoodRegisterAsync *URedwoodRegisterAsync::Register(
  UObject *WorldContextObject,
  ARedwoodTitlePlayerController *PlayerController,
  const FString &Username,
  const FString &Password
) {
  URedwoodRegisterAsync *Action = NewObject<URedwoodRegisterAsync>();
  Action->PlayerController = PlayerController;
  Action->Username = Username;
  Action->Password = Password;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodRegisterAsync::Activate() {
  FRedwoodAuthUpdate Delegate;
  Delegate.BindDynamic(this, &URedwoodRegisterAsync::HandleUpdated);
  PlayerController->Register(Username, Password, Delegate);
}

void URedwoodRegisterAsync::HandleUpdated(
  ERedwoodAuthUpdateType Type, FString Message
) {
  Updated.Broadcast(Type, Message);
}

// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodRegisterUsernameAsync.h"

URedwoodRegisterUsernameAsync *URedwoodRegisterUsernameAsync::RegisterUsername(
  UObject *WorldContextObject,
  ARedwoodTitlePlayerController *PlayerController,
  const FString &Username,
  const FString &Password
) {
  URedwoodRegisterUsernameAsync *Action =
    NewObject<URedwoodRegisterUsernameAsync>();
  Action->PlayerController = PlayerController;
  Action->Username = Username;
  Action->Password = Password;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodRegisterUsernameAsync::Activate() {
  FRedwoodAuthUpdate Delegate;
  Delegate.AddDynamic(this, &URedwoodRegisterUsernameAsync::HandleUpdated);
  PlayerController->RegisterUsername(Username, Password, Delegate);
}

void URedwoodRegisterUsernameAsync::HandleUpdated(
  ERedwoodAuthUpdateType Type, FString Message
) {
  Updated.Broadcast(Type, Message);
}

// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodRegisterEmailAsync.h"

URedwoodRegisterEmailAsync *URedwoodRegisterEmailAsync::RegisterEmail(
  UObject *WorldContextObject,
  ARedwoodTitlePlayerController *PlayerController,
  const FString &Email,
  const FString &Password,
  const FString &DisplayName
) {
  URedwoodRegisterEmailAsync *Action = NewObject<URedwoodRegisterEmailAsync>();
  Action->PlayerController = PlayerController;
  Action->Email = Email;
  Action->Password = Password;
  Action->DisplayName = DisplayName;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodRegisterEmailAsync::Activate() {
  FRedwoodAuthUpdate Delegate;
  Delegate.AddDynamic(this, &URedwoodRegisterEmailAsync::HandleUpdated);
  PlayerController->RegisterEmail(Email, DisplayName, Password, Delegate);
}

void URedwoodRegisterEmailAsync::HandleUpdated(
  ERedwoodAuthUpdateType Type, FString Message
) {
  Updated.Broadcast(Type, Message);
}

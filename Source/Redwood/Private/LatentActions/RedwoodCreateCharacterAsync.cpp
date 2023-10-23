// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodCreateCharacterAsync.h"

URedwoodCreateCharacterAsync *URedwoodCreateCharacterAsync::CreateCharacter(
  UObject *WorldContextObject,
  ARedwoodTitlePlayerController *PlayerController,
  USIOJsonObject *Data
) {
  URedwoodCreateCharacterAsync *Action =
    NewObject<URedwoodCreateCharacterAsync>();
  Action->PlayerController = PlayerController;
  Action->Data = Data;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodCreateCharacterAsync::Activate() {
  FRedwoodCharacterResponse Delegate;
  Delegate.BindDynamic(this, &URedwoodCreateCharacterAsync::HandleResponse);

  PlayerController->CreateCharacter(Data, Delegate);
}

void URedwoodCreateCharacterAsync::HandleResponse(
  FString Error, FRedwoodPlayerCharacter Character
) {
  OnResponse.Broadcast(Error, Character);
}

// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodSetCharacterDataAsync.h"

URedwoodSetCharacterDataAsync *URedwoodSetCharacterDataAsync::SetCharacterData(
  UObject *WorldContextObject,
  ARedwoodTitlePlayerController *PlayerController,
  FString CharacterId,
  USIOJsonObject *Data
) {
  URedwoodSetCharacterDataAsync *Action =
    NewObject<URedwoodSetCharacterDataAsync>();
  Action->PlayerController = PlayerController;
  Action->CharacterId = CharacterId;
  Action->Data = Data;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodSetCharacterDataAsync::Activate() {
  FRedwoodCharacterResponse Delegate;
  Delegate.BindDynamic(this, &URedwoodSetCharacterDataAsync::HandleResponse);

  PlayerController->SetCharacterData(CharacterId, Data, Delegate);
}

void URedwoodSetCharacterDataAsync::HandleResponse(
  FString Error, FRedwoodPlayerCharacter Character
) {
  OnResponse.Broadcast(Error, Character);
}

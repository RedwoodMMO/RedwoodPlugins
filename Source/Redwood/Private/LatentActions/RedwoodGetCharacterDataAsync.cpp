// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodGetCharacterDataAsync.h"

URedwoodGetCharacterDataAsync *URedwoodGetCharacterDataAsync::GetCharacterData(
  UObject *WorldContextObject,
  ARedwoodTitlePlayerController *PlayerController,
  FString CharacterId
) {
  URedwoodGetCharacterDataAsync *Action =
    NewObject<URedwoodGetCharacterDataAsync>();
  Action->PlayerController = PlayerController;
  Action->CharacterId = CharacterId;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodGetCharacterDataAsync::Activate() {
  FRedwoodCharacterResponse Delegate;
  Delegate.BindDynamic(this, &URedwoodGetCharacterDataAsync::HandleResponse);
  PlayerController->GetCharacterData(CharacterId, Delegate);
}

void URedwoodGetCharacterDataAsync::HandleResponse(
  FString Error, FRedwoodPlayerCharacter Character
) {
  OnResponse.Broadcast(Error, Character);
}

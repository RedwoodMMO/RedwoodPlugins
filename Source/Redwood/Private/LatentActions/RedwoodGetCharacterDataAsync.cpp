// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodGetCharacterDataAsync.h"

URedwoodGetCharacterDataAsync *URedwoodGetCharacterDataAsync::GetCharacterData(
  UObject *WorldContextObject, ARedwoodTitlePlayerController *PlayerController
) {
  URedwoodGetCharacterDataAsync *Action =
    NewObject<URedwoodGetCharacterDataAsync>();
  Action->PlayerController = PlayerController;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodGetCharacterDataAsync::Activate() {
  FRedwoodCharacterResponse Delegate;
  Delegate.AddDynamic(this, &URedwoodGetCharacterDataAsync::HandleResponse);
  PlayerController->GetCharacterData(Delegate);
}

void URedwoodGetCharacterDataAsync::HandleResponse(
  FString Error, USIOJsonObject *CharacterData
) {
  OnResponse.Broadcast(Error, CharacterData);
}

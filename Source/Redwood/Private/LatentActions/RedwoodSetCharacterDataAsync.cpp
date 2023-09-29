// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodSetCharacterDataAsync.h"

URedwoodSetCharacterDataAsync *URedwoodSetCharacterDataAsync::SetCharacterData(
  UObject *WorldContextObject,
  ARedwoodTitlePlayerController *PlayerController,
  USIOJsonObject *Data
) {
  URedwoodSetCharacterDataAsync *Action =
    NewObject<URedwoodSetCharacterDataAsync>();
  Action->PlayerController = PlayerController;
  Action->Data = Data;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodSetCharacterDataAsync::Activate() {
  FRedwoodCharacterResponse Delegate;
  Delegate.AddDynamic(this, &URedwoodSetCharacterDataAsync::HandleResponse);
  PlayerController->SetCharacterData(Data->GetRootObject(), Delegate);
}

void URedwoodSetCharacterDataAsync::HandleResponse(
  FString Error, USIOJsonObject *CharacterData
) {
  OnResponse.Broadcast(Error, CharacterData);
}

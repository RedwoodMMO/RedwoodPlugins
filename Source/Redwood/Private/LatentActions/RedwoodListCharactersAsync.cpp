// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodListCharactersAsync.h"

URedwoodListCharactersAsync *URedwoodListCharactersAsync::ListCharacters(
  UObject *WorldContextObject, ARedwoodTitlePlayerController *PlayerController
) {
  URedwoodListCharactersAsync *Action =
    NewObject<URedwoodListCharactersAsync>();
  Action->PlayerController = PlayerController;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodListCharactersAsync::Activate() {
  FRedwoodCharactersResponse Delegate;
  Delegate.BindDynamic(this, &URedwoodListCharactersAsync::HandleResponse);

  PlayerController->ListCharacters(Delegate);
}

void URedwoodListCharactersAsync::HandleResponse(
  FString Error, const TArray<FRedwoodPlayerCharacter> &Characters
) {
  OnResponse.Broadcast(Error, Characters);
}

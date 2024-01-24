// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodListCharactersAsync.h"

URedwoodListCharactersAsync *URedwoodListCharactersAsync::ListCharacters(
  URedwoodTitleGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodListCharactersAsync *Action =
    NewObject<URedwoodListCharactersAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodListCharactersAsync::Activate() {
  Target->ListCharacters(
    URedwoodTitleGameSubsystem::FRedwoodOnListCharacters::CreateLambda(
      [this](FRedwoodCharactersResult Result) {
        OnResult.Broadcast(Result);
        SetReadyToDestroy();
      }
    )
  );
}

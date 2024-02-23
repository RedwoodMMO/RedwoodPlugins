// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodListCharactersAsync.h"

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
  Target->ListCharacters(FRedwoodListCharactersOutputDelegate::CreateLambda(
    [this](FRedwoodListCharactersOutput Output) {
      OnOutput.Broadcast(Output);
      SetReadyToDestroy();
    }
  ));
}

// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodListArchivedCharactersAsync.h"

URedwoodListArchivedCharactersAsync *
URedwoodListArchivedCharactersAsync::ListArchivedCharacters(
  URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodListArchivedCharactersAsync *Action =
    NewObject<URedwoodListArchivedCharactersAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodListArchivedCharactersAsync::Activate() {
  Target->ListArchivedCharacters(
    FRedwoodListCharactersOutputDelegate::CreateLambda(
      [this](FRedwoodListCharactersOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

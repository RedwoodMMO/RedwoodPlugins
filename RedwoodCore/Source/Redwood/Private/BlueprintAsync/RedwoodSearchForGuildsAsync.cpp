// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodSearchForGuildsAsync.h"

URedwoodSearchForGuildsAsync *URedwoodSearchForGuildsAsync::SearchForGuilds(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString SearchText,
  bool bIncludePartialMatches
) {
  URedwoodSearchForGuildsAsync *Action =
    NewObject<URedwoodSearchForGuildsAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->SearchText = SearchText;
  Action->bIncludePartialMatches = bIncludePartialMatches;

  return Action;
}

void URedwoodSearchForGuildsAsync::Activate() {
  Target->SearchForGuilds(
    SearchText,
    bIncludePartialMatches,
    FRedwoodListGuildsOutputDelegate::CreateLambda(
      [this](FRedwoodListGuildsOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}
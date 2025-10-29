// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodSearchForAlliancesAsync.h"

URedwoodSearchForAlliancesAsync *
URedwoodSearchForAlliancesAsync::SearchForAlliances(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString SearchText,
  bool bIncludePartialMatches
) {
  URedwoodSearchForAlliancesAsync *Action =
    NewObject<URedwoodSearchForAlliancesAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->SearchText = SearchText;
  Action->bIncludePartialMatches = bIncludePartialMatches;

  return Action;
}

void URedwoodSearchForAlliancesAsync::Activate() {
  Target->SearchForAlliances(
    SearchText,
    bIncludePartialMatches,
    FRedwoodListAlliancesOutputDelegate::CreateLambda(
      [this](FRedwoodListAlliancesOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}
// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodListGuildsAsync.h"

URedwoodListGuildsAsync *URedwoodListGuildsAsync::ListGuilds(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  bool bOnlyPlayersGuilds
) {
  URedwoodListGuildsAsync *Action = NewObject<URedwoodListGuildsAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->bOnlyPlayersGuilds = bOnlyPlayersGuilds;

  return Action;
}

void URedwoodListGuildsAsync::Activate() {
  Target->ListGuilds(
    bOnlyPlayersGuilds,
    FRedwoodListGuildsOutputDelegate::CreateLambda(
      [this](FRedwoodListGuildsOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}
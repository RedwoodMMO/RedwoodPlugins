// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodListAlliancesAsync.h"

URedwoodListAlliancesAsync *URedwoodListAlliancesAsync::ListAlliances(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString GuildIdFilter
) {
  URedwoodListAlliancesAsync *Action = NewObject<URedwoodListAlliancesAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->GuildIdFilter = GuildIdFilter;

  return Action;
}

void URedwoodListAlliancesAsync::Activate() {
  Target->ListAlliances(
    GuildIdFilter,
    FRedwoodListAlliancesOutputDelegate::CreateLambda(
      [this](FRedwoodListAlliancesOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}
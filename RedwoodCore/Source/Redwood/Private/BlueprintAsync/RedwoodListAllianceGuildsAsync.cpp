// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodListAllianceGuildsAsync.h"

URedwoodListAllianceGuildsAsync *
URedwoodListAllianceGuildsAsync::ListAllianceGuilds(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString AllianceId,
  ERedwoodGuildAndAllianceMemberState State
) {
  URedwoodListAllianceGuildsAsync *Action =
    NewObject<URedwoodListAllianceGuildsAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->AllianceId = AllianceId;
  Action->State = State;

  return Action;
}

void URedwoodListAllianceGuildsAsync::Activate() {
  Target->ListAllianceGuilds(
    AllianceId,
    State,
    FRedwoodListAllianceGuildsOutputDelegate::CreateLambda(
      [this](FRedwoodListAllianceGuildsOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}
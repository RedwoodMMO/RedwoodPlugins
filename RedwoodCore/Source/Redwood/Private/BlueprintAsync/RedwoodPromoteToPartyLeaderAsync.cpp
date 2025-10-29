// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodPromoteToPartyLeaderAsync.h"

URedwoodPromoteToPartyLeaderAsync *
URedwoodPromoteToPartyLeaderAsync::PromoteToPartyLeader(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString TargetPlayerId
) {
  URedwoodPromoteToPartyLeaderAsync *Action =
    NewObject<URedwoodPromoteToPartyLeaderAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->TargetPlayerId = TargetPlayerId;

  return Action;
}

void URedwoodPromoteToPartyLeaderAsync::Activate() {
  Target->PromoteToPartyLeader(
    TargetPlayerId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Output) {
      OnOutput.Broadcast(Output);
      SetReadyToDestroy();
    })
  );
}
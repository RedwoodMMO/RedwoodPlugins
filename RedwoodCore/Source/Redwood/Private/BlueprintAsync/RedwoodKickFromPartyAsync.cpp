// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodKickFromPartyAsync.h"

URedwoodKickFromPartyAsync *URedwoodKickFromPartyAsync::KickFromParty(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString TargetPlayerId
) {
  URedwoodKickFromPartyAsync *Action = NewObject<URedwoodKickFromPartyAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->TargetPlayerId = TargetPlayerId;

  return Action;
}

void URedwoodKickFromPartyAsync::Activate() {
  Target->KickFromParty(
    TargetPlayerId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Output) {
      OnOutput.Broadcast(Output);
      SetReadyToDestroy();
    })
  );
}
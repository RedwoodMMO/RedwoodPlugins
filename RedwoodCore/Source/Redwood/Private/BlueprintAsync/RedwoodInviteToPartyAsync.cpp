// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodInviteToPartyAsync.h"

URedwoodInviteToPartyAsync *URedwoodInviteToPartyAsync::InviteToParty(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString TargetPlayerId
) {
  URedwoodInviteToPartyAsync *Action = NewObject<URedwoodInviteToPartyAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->TargetPlayerId = TargetPlayerId;

  return Action;
}

void URedwoodInviteToPartyAsync::Activate() {
  Target->InviteToParty(
    TargetPlayerId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Output) {
      OnOutput.Broadcast(Output);
      SetReadyToDestroy();
    })
  );
}
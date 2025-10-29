// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodListPartyInvitesAsync.h"

URedwoodListPartyInvitesAsync *URedwoodListPartyInvitesAsync::ListPartyInvites(
  URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodListPartyInvitesAsync *Action =
    NewObject<URedwoodListPartyInvitesAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodListPartyInvitesAsync::Activate() {
  Target->ListPartyInvites(FRedwoodListPartyInvitesOutputDelegate::CreateLambda(
    [this](FRedwoodListPartyInvitesOutput Output) {
      OnOutput.Broadcast(Output);
      SetReadyToDestroy();
    }
  ));
}
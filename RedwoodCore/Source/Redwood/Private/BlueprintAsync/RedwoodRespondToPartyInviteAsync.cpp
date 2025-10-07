// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodRespondToPartyInviteAsync.h"

URedwoodRespondToPartyInviteAsync *
URedwoodRespondToPartyInviteAsync::RespondToPartyInvite(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString PartyId,
  bool bAccept
) {
  URedwoodRespondToPartyInviteAsync *Action =
    NewObject<URedwoodRespondToPartyInviteAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->PartyId = PartyId;
  Action->bAccept = bAccept;

  return Action;
}

void URedwoodRespondToPartyInviteAsync::Activate() {
  Target->RespondToPartyInvite(
    PartyId,
    bAccept,
    FRedwoodGetPartyOutputDelegate::CreateLambda(
      [this](FRedwoodGetPartyOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}
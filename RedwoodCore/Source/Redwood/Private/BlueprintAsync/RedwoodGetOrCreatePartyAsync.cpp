// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodGetOrCreatePartyAsync.h"

URedwoodGetOrCreatePartyAsync *URedwoodGetOrCreatePartyAsync::GetOrCreateParty(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  bool bCreateIfNotInParty
) {
  URedwoodGetOrCreatePartyAsync *Action =
    NewObject<URedwoodGetOrCreatePartyAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->bCreateIfNotInParty = bCreateIfNotInParty;

  return Action;
}

void URedwoodGetOrCreatePartyAsync::Activate() {
  Target->GetOrCreateParty(
    bCreateIfNotInParty,
    FRedwoodGetPartyOutputDelegate::CreateLambda(
      [this](FRedwoodGetPartyOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}
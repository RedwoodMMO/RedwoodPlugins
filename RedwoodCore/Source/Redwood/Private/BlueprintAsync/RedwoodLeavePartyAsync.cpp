// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodLeavePartyAsync.h"

URedwoodLeavePartyAsync *URedwoodLeavePartyAsync::LeaveParty(
  URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodLeavePartyAsync *Action = NewObject<URedwoodLeavePartyAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodLeavePartyAsync::Activate() {
  Target->LeaveParty(
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Output) {
      OnOutput.Broadcast(Output);
      SetReadyToDestroy();
    })
  );
}
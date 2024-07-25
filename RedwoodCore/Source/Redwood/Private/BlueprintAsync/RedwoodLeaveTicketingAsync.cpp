// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodLeaveTicketingAsync.h"

URedwoodLeaveTicketingAsync *URedwoodLeaveTicketingAsync::LeaveTicketing(
  URedwoodTitleGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodLeaveTicketingAsync *Action =
    NewObject<URedwoodLeaveTicketingAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodLeaveTicketingAsync::Activate() {
  Target->LeaveTicketing(
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Output) {
      OnOutput.Broadcast(Output);

      SetReadyToDestroy();
    })
  );
}

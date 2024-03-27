// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodCancelTicketingAsync.h"

URedwoodCancelTicketingAsync *URedwoodCancelTicketingAsync::CancelTicketing(
  URedwoodTitleGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodCancelTicketingAsync *Action =
    NewObject<URedwoodCancelTicketingAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodCancelTicketingAsync::Activate() {
  Target->CancelTicketing(
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Output) {
      OnOutput.Broadcast(Output);

      SetReadyToDestroy();
    })
  );
}

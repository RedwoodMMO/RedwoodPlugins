// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodGetSelectedGuildAsync.h"

URedwoodGetSelectedGuildAsync *URedwoodGetSelectedGuildAsync::GetSelectedGuild(
  URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodGetSelectedGuildAsync *Action =
    NewObject<URedwoodGetSelectedGuildAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodGetSelectedGuildAsync::Activate() {
  Target->GetSelectedGuild(FRedwoodGetGuildOutputDelegate::CreateLambda(
    [this](FRedwoodGetGuildOutput Output) {
      OnOutput.Broadcast(Output);
      SetReadyToDestroy();
    }
  ));
}
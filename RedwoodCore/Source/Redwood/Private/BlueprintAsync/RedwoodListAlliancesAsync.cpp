// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodListAlliancesAsync.h"

URedwoodListAlliancesAsync *URedwoodListAlliancesAsync::ListAlliances(
  URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodListAlliancesAsync *Action = NewObject<URedwoodListAlliancesAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodListAlliancesAsync::Activate() {
  Target->ListAlliances(FRedwoodListAlliancesOutputDelegate::CreateLambda(
    [this](FRedwoodListAlliancesOutput Output) {
      OnOutput.Broadcast(Output);
      SetReadyToDestroy();
    }
  ));
}
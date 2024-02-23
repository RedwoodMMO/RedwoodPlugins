// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodListRealmsAsync.h"

URedwoodListRealmsAsync *URedwoodListRealmsAsync::ListRealms(
  URedwoodTitleGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodListRealmsAsync *Action = NewObject<URedwoodListRealmsAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodListRealmsAsync::Activate() {
  Target->ListRealms(FRedwoodListRealmsOutputDelegate::CreateLambda(
    [this](FRedwoodListRealmsOutput Output) {
      OnOutput.Broadcast(Output);
      SetReadyToDestroy();
    }
  ));
}

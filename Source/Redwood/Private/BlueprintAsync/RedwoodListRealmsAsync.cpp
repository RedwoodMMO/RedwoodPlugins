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
  Target->ListRealms(
    URedwoodTitleGameSubsystem::FRedwoodOnListRealms::CreateLambda(
      [this](FRedwoodRealmsResult Result) {
        OnResult.Broadcast(Result);
        SetReadyToDestroy();
      }
    )
  );
}

// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodListServersAsync.h"

URedwoodListServersAsync *URedwoodListServersAsync::ListServers(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  TArray<FString> PrivateServerReferences
) {
  URedwoodListServersAsync *Action = NewObject<URedwoodListServersAsync>();
  Action->Target = Target;
  Action->PrivateServerReferences = PrivateServerReferences;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodListServersAsync::Activate() {
  Target->ListServers(
    PrivateServerReferences,
    FRedwoodListServersOutputDelegate::CreateLambda(
      [this](FRedwoodListServersOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

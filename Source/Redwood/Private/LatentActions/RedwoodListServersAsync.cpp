// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodListServersAsync.h"

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
    URedwoodTitleGameSubsystem::FRedwoodOnListServers::CreateLambda(
      [this](FRedwoodListServers Result) {
        OnResult.Broadcast(Result);
        SetReadyToDestroy();
      }
    )
  );
}

// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodCreateServerAsync.h"

URedwoodCreateServerAsync *URedwoodCreateServerAsync::CreateServer(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  FRedwoodCreateServerParameters Parameters
) {
  URedwoodCreateServerAsync *Action = NewObject<URedwoodCreateServerAsync>();
  Action->Target = Target;
  Action->Parameters = Parameters;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodCreateServerAsync::Activate() {
  Target->CreateServer(
    Parameters,
    URedwoodTitleGameSubsystem::FRedwoodOnGetServer::CreateLambda(
      [this](FRedwoodGetServer Result) {
        OnResult.Broadcast(Result);
        SetReadyToDestroy();
      }
    )
  );
}

// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodCreateServerAsync.h"

URedwoodCreateServerAsync *URedwoodCreateServerAsync::CreateServer(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  bool bJoinSession,
  FRedwoodCreateServerInput Parameters
) {
  URedwoodCreateServerAsync *Action = NewObject<URedwoodCreateServerAsync>();
  Action->Target = Target;
  Action->bJoinSession = bJoinSession;
  Action->Parameters = Parameters;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodCreateServerAsync::Activate() {
  Target->CreateServer(
    bJoinSession,
    Parameters,
    FRedwoodCreateServerOutputDelegate::CreateLambda(
      [this](FRedwoodCreateServerOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

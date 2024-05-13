// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodJoinServerInstanceAsync.h"

URedwoodJoinServerInstanceAsync *
URedwoodJoinServerInstanceAsync::JoinServerInstance(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  FString ServerReference,
  FString Password
) {
  URedwoodJoinServerInstanceAsync *Action =
    NewObject<URedwoodJoinServerInstanceAsync>();
  Action->Target = Target;
  Action->ServerReference = ServerReference;
  Action->Password = Password;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodJoinServerInstanceAsync::Activate() {
  Target->JoinServerInstance(
    ServerReference,
    Password,
    FRedwoodJoinServerOutputDelegate::CreateLambda(
      [this](FRedwoodJoinServerOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

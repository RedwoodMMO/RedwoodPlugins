// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodInitializeDirectorConnectionAsync.h"

URedwoodInitializeDirectorConnectionAsync *
URedwoodInitializeDirectorConnectionAsync::InitializeDirectorConnection(
  URedwoodTitleGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodInitializeDirectorConnectionAsync *Action =
    NewObject<URedwoodInitializeDirectorConnectionAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodInitializeDirectorConnectionAsync::Activate() {
  Target->InitializeDirectorConnection(
    FRedwoodSocketConnectedDelegate::CreateLambda(
      [this](FRedwoodSocketConnected Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodInitiateDirectorConnectionAsync.h"

URedwoodInitiateDirectorConnectionAsync *
URedwoodInitiateDirectorConnectionAsync::ListRealms(
  URedwoodTitleGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodInitiateDirectorConnectionAsync *Action =
    NewObject<URedwoodInitiateDirectorConnectionAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodInitiateDirectorConnectionAsync::Activate() {
  Target->InitializeDirectorConnection(
    FRedwoodSocketConnectedDelegate::CreateLambda(
      [this](FRedwoodSocketConnected Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

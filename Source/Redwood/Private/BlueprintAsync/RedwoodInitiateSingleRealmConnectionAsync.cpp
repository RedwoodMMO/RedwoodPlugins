// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodInitiateSingleRealmConnectionAsync.h"

URedwoodInitiateSingleRealmConnectionAsync *
URedwoodInitiateSingleRealmConnectionAsync::InitializeSingleRealmConnection(
  URedwoodTitleGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodInitiateSingleRealmConnectionAsync *Action =
    NewObject<URedwoodInitiateSingleRealmConnectionAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodInitiateSingleRealmConnectionAsync::Activate() {
  Target->InitializeSingleRealmConnection(
    FRedwoodSocketConnectedDelegate::CreateLambda(
      [this](FRedwoodSocketConnected Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

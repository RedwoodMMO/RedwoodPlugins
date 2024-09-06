// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodInitializeConnectionForFirstRealmAsync.h"

URedwoodInitializeConnectionForFirstRealmAsync *
URedwoodInitializeConnectionForFirstRealmAsync::
  InitializeConnectionForFirstRealm(
    URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
  ) {
  URedwoodInitializeConnectionForFirstRealmAsync *Action =
    NewObject<URedwoodInitializeConnectionForFirstRealmAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodInitializeConnectionForFirstRealmAsync::Activate() {
  Target->InitializeConnectionForFirstRealm(
    FRedwoodSocketConnectedDelegate::CreateLambda(
      [this](FRedwoodSocketConnected Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

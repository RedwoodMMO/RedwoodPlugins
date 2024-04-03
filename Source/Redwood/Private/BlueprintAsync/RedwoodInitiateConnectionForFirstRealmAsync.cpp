// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodInitiateConnectionForFirstRealmAsync.h"

URedwoodInitiateConnectionForFirstRealmAsync *
URedwoodInitiateConnectionForFirstRealmAsync::InitializeConnectionForFirstRealm(
  URedwoodTitleGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodInitiateConnectionForFirstRealmAsync *Action =
    NewObject<URedwoodInitiateConnectionForFirstRealmAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodInitiateConnectionForFirstRealmAsync::Activate() {
  Target->InitializeConnectionForFirstRealm(
    FRedwoodSocketConnectedDelegate::CreateLambda(
      [this](FRedwoodSocketConnected Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

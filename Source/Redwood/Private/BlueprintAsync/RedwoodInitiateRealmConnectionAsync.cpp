// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodInitiateRealmConnectionAsync.h"

URedwoodInitiateRealmConnectionAsync *
URedwoodInitiateRealmConnectionAsync::InitializeRealmConnection(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  FRedwoodRealm Realm
) {
  URedwoodInitiateRealmConnectionAsync *Action =
    NewObject<URedwoodInitiateRealmConnectionAsync>();
  Action->Target = Target;
  Action->Realm = Realm;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodInitiateRealmConnectionAsync::Activate() {
  Target->InitializeRealmConnection(
    Realm,
    FRedwoodSocketConnectedDelegate::CreateLambda(
      [this](FRedwoodSocketConnected Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

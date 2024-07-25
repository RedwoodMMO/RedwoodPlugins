// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodInitializeRealmConnectionAsync.h"

URedwoodInitializeRealmConnectionAsync *
URedwoodInitializeRealmConnectionAsync::InitializeRealmConnection(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  FRedwoodRealm Realm
) {
  URedwoodInitializeRealmConnectionAsync *Action =
    NewObject<URedwoodInitializeRealmConnectionAsync>();
  Action->Target = Target;
  Action->Realm = Realm;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodInitializeRealmConnectionAsync::Activate() {
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

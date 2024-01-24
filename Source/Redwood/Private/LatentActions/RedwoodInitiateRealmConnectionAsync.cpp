// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodInitiateRealmConnectionAsync.h"

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
    URedwoodTitleGameSubsystem::FRedwoodOnSocketConnected::CreateLambda(
      [this](FRedwoodSocketConnected Result) {
        OnResult.Broadcast(Result);
        SetReadyToDestroy();
      }
    )
  );
}

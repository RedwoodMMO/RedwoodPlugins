// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodInitiateSingleRealmConnectionAsync.h"

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
    URedwoodTitleGameSubsystem::FRedwoodOnSocketConnected::CreateLambda(
      [this](FRedwoodSocketConnected Result) {
        OnResult.Broadcast(Result);
        SetReadyToDestroy();
      }
    )
  );
}

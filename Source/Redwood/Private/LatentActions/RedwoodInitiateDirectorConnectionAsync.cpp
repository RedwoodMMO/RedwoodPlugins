// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodInitiateDirectorConnectionAsync.h"

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
    URedwoodTitleGameSubsystem::FRedwoodOnSocketConnected::CreateLambda(
      [this](FRedwoodSocketConnected Result) {
        OnResult.Broadcast(Result);
        SetReadyToDestroy();
      }
    )
  );
}

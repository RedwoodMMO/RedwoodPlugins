// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodGetServerInstanceAsync.h"

URedwoodGetServerInstanceAsync *
URedwoodGetServerInstanceAsync::GetServerInstance(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  FString ServerReference,
  FString Password
) {
  URedwoodGetServerInstanceAsync *Action =
    NewObject<URedwoodGetServerInstanceAsync>();
  Action->Target = Target;
  Action->ServerReference = ServerReference;
  Action->Password = Password;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodGetServerInstanceAsync::Activate() {
  Target->GetServerInstance(
    ServerReference,
    Password,
    URedwoodTitleGameSubsystem::FRedwoodOnGetServer::CreateLambda(
      [this](FRedwoodGetServer Result) {
        OnResult.Broadcast(Result);
        SetReadyToDestroy();
      }
    )
  );
}

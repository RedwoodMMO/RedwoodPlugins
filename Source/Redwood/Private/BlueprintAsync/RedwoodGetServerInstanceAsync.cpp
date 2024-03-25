// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodGetServerInstanceAsync.h"

URedwoodGetServerInstanceAsync *
URedwoodGetServerInstanceAsync::GetServerInstance(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  FString ServerReference,
  FString Password,
  bool bJoinSession
) {
  URedwoodGetServerInstanceAsync *Action =
    NewObject<URedwoodGetServerInstanceAsync>();
  Action->Target = Target;
  Action->ServerReference = ServerReference;
  Action->Password = Password;
  Action->bJoinSession = bJoinSession;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodGetServerInstanceAsync::Activate() {
  Target->GetServerInstance(
    ServerReference,
    Password,
    bJoinSession,
    FRedwoodGetServerOutputDelegate::CreateLambda(
      [this](FRedwoodGetServerOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

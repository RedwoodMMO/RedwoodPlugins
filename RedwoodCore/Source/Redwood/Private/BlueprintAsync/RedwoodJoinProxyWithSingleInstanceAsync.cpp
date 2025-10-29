// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodJoinProxyWithSingleInstanceAsync.h"

URedwoodJoinProxyWithSingleInstanceAsync *
URedwoodJoinProxyWithSingleInstanceAsync::JoinProxyWithSingleInstance(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString ProxyReference,
  FString Password
) {
  URedwoodJoinProxyWithSingleInstanceAsync *Action =
    NewObject<URedwoodJoinProxyWithSingleInstanceAsync>();
  Action->Target = Target;
  Action->ProxyReference = ProxyReference;
  Action->Password = Password;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodJoinProxyWithSingleInstanceAsync::Activate() {
  Target->JoinProxyWithSingleInstance(
    ProxyReference,
    Password,
    FRedwoodJoinServerOutputDelegate::CreateLambda(
      [this](FRedwoodJoinServerOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

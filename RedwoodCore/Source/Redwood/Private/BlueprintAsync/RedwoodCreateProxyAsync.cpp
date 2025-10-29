// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodCreateProxyAsync.h"

URedwoodCreateProxyAsync *URedwoodCreateProxyAsync::CreateProxy(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  bool bJoinSession,
  FRedwoodCreateProxyInput Parameters
) {
  URedwoodCreateProxyAsync *Action = NewObject<URedwoodCreateProxyAsync>();
  Action->Target = Target;
  Action->bJoinSession = bJoinSession;
  Action->Parameters = Parameters;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodCreateProxyAsync::Activate() {
  Target->CreateProxy(
    bJoinSession,
    Parameters,
    FRedwoodCreateProxyOutputDelegate::CreateLambda(
      [this](FRedwoodCreateProxyOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

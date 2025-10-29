// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodStopProxyAsync.h"

URedwoodStopProxyAsync *URedwoodStopProxyAsync::StopProxy(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString ServerProxyId
) {
  URedwoodStopProxyAsync *Action = NewObject<URedwoodStopProxyAsync>();
  Action->Target = Target;
  Action->ServerProxyId = ServerProxyId;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodStopProxyAsync::Activate() {
  Target->StopProxy(
    ServerProxyId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}

// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodStopServerAsync.h"

URedwoodStopServerAsync *URedwoodStopServerAsync::StopServer(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString ServerProxyId
) {
  URedwoodStopServerAsync *Action = NewObject<URedwoodStopServerAsync>();
  Action->Target = Target;
  Action->ServerProxyId = ServerProxyId;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodStopServerAsync::Activate() {
  Target->StopServer(
    ServerProxyId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}

// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodStopServerAsync.h"

URedwoodStopServerAsync *URedwoodStopServerAsync::StopServer(
  URedwoodTitleGameSubsystem *Target,
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
    URedwoodTitleGameSubsystem::FRedwoodOnSimpleResult::CreateLambda(
      [this](FString Error) {
        OnResult.Broadcast(Error);
        SetReadyToDestroy();
      }
    )
  );
}

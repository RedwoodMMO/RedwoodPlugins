// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodListProxiesAsync.h"

URedwoodListProxiesAsync *URedwoodListProxiesAsync::ListProxies(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  TArray<FString> PrivateProxyReferences
) {
  URedwoodListProxiesAsync *Action = NewObject<URedwoodListProxiesAsync>();
  Action->Target = Target;
  Action->PrivateProxyReferences = PrivateProxyReferences;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodListProxiesAsync::Activate() {
  Target->ListProxies(
    PrivateProxyReferences,
    FRedwoodListProxiesOutputDelegate::CreateLambda(
      [this](FRedwoodListProxiesOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodGetRealmGlobalDataAsync.h"

URedwoodGetRealmGlobalDataAsync *
URedwoodGetRealmGlobalDataAsync::GetRealmGlobalData(
  URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodGetRealmGlobalDataAsync *Action =
    NewObject<URedwoodGetRealmGlobalDataAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodGetRealmGlobalDataAsync::Activate() {
  Target->GetRealmGlobalData(FRedwoodGetGlobalDataOutputDelegate::CreateLambda(
    [this](FRedwoodGetGlobalDataOutput Output) {
      OnOutput.Broadcast(Output);
      SetReadyToDestroy();
    }
  ));
}

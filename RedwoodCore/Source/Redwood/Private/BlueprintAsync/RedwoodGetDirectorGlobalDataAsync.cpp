// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodGetDirectorGlobalDataAsync.h"

URedwoodGetDirectorGlobalDataAsync *
URedwoodGetDirectorGlobalDataAsync::GetDirectorGlobalData(
  URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodGetDirectorGlobalDataAsync *Action =
    NewObject<URedwoodGetDirectorGlobalDataAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodGetDirectorGlobalDataAsync::Activate() {
  Target->GetDirectorGlobalData(
    FRedwoodGetGlobalDataOutputDelegate::CreateLambda(
      [this](FRedwoodGetGlobalDataOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

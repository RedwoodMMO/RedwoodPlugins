// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodGetSaveGameAsync.h"

URedwoodGetSaveGameAsync *URedwoodGetSaveGameAsync::GetSaveGame(
  URedwoodServerGameSubsystem *Target, UObject *WorldContextObject, FString Key
) {
  URedwoodGetSaveGameAsync *Action = NewObject<URedwoodGetSaveGameAsync>();
  Action->Target = Target;
  Action->Key = Key;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodGetSaveGameAsync::Activate() {
  Target->GetSaveGame(
    Key,
    FRedwoodGetSaveGameOutputDelegate::CreateLambda(
      [this](FRedwoodGetSaveGameOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

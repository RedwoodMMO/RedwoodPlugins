// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodPutSaveGameAsync.h"

URedwoodPutSaveGameAsync *URedwoodPutSaveGameAsync::PutSaveGame(
  URedwoodServerGameSubsystem *Target,
  UObject *WorldContextObject,
  FString Key,
  USaveGame *SaveGame
) {
  URedwoodPutSaveGameAsync *Action = NewObject<URedwoodPutSaveGameAsync>();
  Action->Target = Target;
  Action->Key = Key;
  Action->SaveGame = SaveGame;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodPutSaveGameAsync::Activate() {
  Target->PutSaveGame(
    Key,
    SaveGame,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}

// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodCreateCharacterAsync.h"

URedwoodCreateCharacterAsync *URedwoodCreateCharacterAsync::CreateCharacter(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  USIOJsonObject *Data
) {
  URedwoodCreateCharacterAsync *Action =
    NewObject<URedwoodCreateCharacterAsync>();
  Action->Target = Target;
  Action->Data = Data;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodCreateCharacterAsync::Activate() {
  Target->CreateCharacter(
    Data,
    FRedwoodGetCharacterOutputDelegate::CreateLambda(
      [this](FRedwoodGetCharacterOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

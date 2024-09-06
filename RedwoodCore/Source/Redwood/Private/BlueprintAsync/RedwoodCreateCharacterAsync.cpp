// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodCreateCharacterAsync.h"

URedwoodCreateCharacterAsync *URedwoodCreateCharacterAsync::CreateCharacter(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString Name,
  USIOJsonObject *CharacterCreatorData
) {
  URedwoodCreateCharacterAsync *Action =
    NewObject<URedwoodCreateCharacterAsync>();
  Action->Target = Target;
  Action->Name = Name;
  Action->CharacterCreatorData = CharacterCreatorData;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodCreateCharacterAsync::Activate() {
  Target->CreateCharacter(
    Name,
    CharacterCreatorData,
    FRedwoodGetCharacterOutputDelegate::CreateLambda(
      [this](FRedwoodGetCharacterOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

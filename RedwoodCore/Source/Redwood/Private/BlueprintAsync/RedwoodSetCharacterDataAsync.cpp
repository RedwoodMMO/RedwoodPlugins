// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodSetCharacterDataAsync.h"

URedwoodSetCharacterDataAsync *URedwoodSetCharacterDataAsync::SetCharacterData(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString CharacterId,
  FString Name,
  USIOJsonObject *CharacterCreatorData
) {
  URedwoodSetCharacterDataAsync *Action =
    NewObject<URedwoodSetCharacterDataAsync>();
  Action->Target = Target;
  Action->CharacterId = CharacterId;
  Action->Name = Name;
  Action->CharacterCreatorData = CharacterCreatorData;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodSetCharacterDataAsync::Activate() {
  Target->SetCharacterData(
    CharacterId,
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

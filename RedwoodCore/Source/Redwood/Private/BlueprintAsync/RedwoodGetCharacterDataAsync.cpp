// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodGetCharacterDataAsync.h"

URedwoodGetCharacterDataAsync *URedwoodGetCharacterDataAsync::GetCharacterData(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString CharacterIdOrName
) {
  URedwoodGetCharacterDataAsync *Action =
    NewObject<URedwoodGetCharacterDataAsync>();
  Action->Target = Target;
  Action->CharacterIdOrName = CharacterIdOrName;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodGetCharacterDataAsync::Activate() {
  Target->GetCharacterData(
    CharacterIdOrName,
    FRedwoodGetCharacterOutputDelegate::CreateLambda(
      [this](FRedwoodGetCharacterOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

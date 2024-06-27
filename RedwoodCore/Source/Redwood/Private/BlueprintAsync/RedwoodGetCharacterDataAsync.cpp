// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodGetCharacterDataAsync.h"

URedwoodGetCharacterDataAsync *URedwoodGetCharacterDataAsync::GetCharacterData(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  FString CharacterId
) {
  URedwoodGetCharacterDataAsync *Action =
    NewObject<URedwoodGetCharacterDataAsync>();
  Action->Target = Target;
  Action->CharacterId = CharacterId;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodGetCharacterDataAsync::Activate() {
  Target->GetCharacterData(
    CharacterId,
    FRedwoodGetCharacterOutputDelegate::CreateLambda(
      [this](FRedwoodGetCharacterOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

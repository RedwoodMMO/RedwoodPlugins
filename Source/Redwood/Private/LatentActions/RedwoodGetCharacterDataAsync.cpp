// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodGetCharacterDataAsync.h"

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
    URedwoodTitleGameSubsystem::FRedwoodOnGetCharacter::CreateLambda(
      [this](FRedwoodCharacterResult Result) {
        OnResult.Broadcast(Result);
        SetReadyToDestroy();
      }
    )
  );
}

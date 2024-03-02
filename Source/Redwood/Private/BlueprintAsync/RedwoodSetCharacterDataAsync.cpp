// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodSetCharacterDataAsync.h"

URedwoodSetCharacterDataAsync *URedwoodSetCharacterDataAsync::SetCharacterData(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  FString CharacterId,
  USIOJsonObject *Metadata,
  USIOJsonObject *EquippedInventory,
  USIOJsonObject *NonequippedInventory,
  USIOJsonObject *Data
) {
  URedwoodSetCharacterDataAsync *Action =
    NewObject<URedwoodSetCharacterDataAsync>();
  Action->Target = Target;
  Action->CharacterId = CharacterId;
  Action->Metadata = Metadata;
  Action->EquippedInventory = EquippedInventory;
  Action->NonequippedInventory = NonequippedInventory;
  Action->Data = Data;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodSetCharacterDataAsync::Activate() {
  Target->SetCharacterData(
    CharacterId,
    Metadata,
    EquippedInventory,
    NonequippedInventory,
    Data,
    FRedwoodGetCharacterOutputDelegate::CreateLambda(
      [this](FRedwoodGetCharacterOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

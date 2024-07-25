// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodCreateCharacterAsync.h"

URedwoodCreateCharacterAsync *URedwoodCreateCharacterAsync::CreateCharacter(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  USIOJsonObject *Metadata,
  USIOJsonObject *EquippedInventory,
  USIOJsonObject *NonequippedInventory,
  USIOJsonObject *Data
) {
  URedwoodCreateCharacterAsync *Action =
    NewObject<URedwoodCreateCharacterAsync>();
  Action->Target = Target;
  Action->Metadata = Metadata;
  Action->EquippedInventory = EquippedInventory;
  Action->NonequippedInventory = NonequippedInventory;
  Action->Data = Data;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodCreateCharacterAsync::Activate() {
  Target->CreateCharacter(
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

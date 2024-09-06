// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodCreateCharacterAsync.h"

URedwoodCreateCharacterAsync *URedwoodCreateCharacterAsync::CreateCharacter(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString Name,
  USIOJsonObject *Metadata,
  USIOJsonObject *EquippedInventory,
  USIOJsonObject *NonequippedInventory,
  USIOJsonObject *Data
) {
  URedwoodCreateCharacterAsync *Action =
    NewObject<URedwoodCreateCharacterAsync>();
  Action->Target = Target;
  Action->Name = Name;
  Action->Metadata = Metadata;
  Action->EquippedInventory = EquippedInventory;
  Action->NonequippedInventory = NonequippedInventory;
  Action->Data = Data;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodCreateCharacterAsync::Activate() {
  Target->CreateCharacter(
    Name,
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

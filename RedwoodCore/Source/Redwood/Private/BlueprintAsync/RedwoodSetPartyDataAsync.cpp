// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodSetPartyDataAsync.h"

URedwoodSetPartyDataAsync *URedwoodSetPartyDataAsync::SetPartyData(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString LootType,
  USIOJsonObject *PartyData
) {
  URedwoodSetPartyDataAsync *Action = NewObject<URedwoodSetPartyDataAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->LootType = LootType;
  Action->PartyData = PartyData;

  return Action;
}

void URedwoodSetPartyDataAsync::Activate() {
  Target->SetPartyData(
    LootType,
    PartyData,
    FRedwoodGetPartyOutputDelegate::CreateLambda(
      [this](FRedwoodGetPartyOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}
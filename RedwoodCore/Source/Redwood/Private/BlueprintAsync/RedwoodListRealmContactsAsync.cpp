// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodListRealmContactsAsync.h"

URedwoodListRealmContactsAsync *
URedwoodListRealmContactsAsync::ListRealmContacts(
  URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodListRealmContactsAsync *Action =
    NewObject<URedwoodListRealmContactsAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodListRealmContactsAsync::Activate() {
  Target->ListRealmContacts(
    FRedwoodListRealmContactsOutputDelegate::CreateLambda(
      [this](FRedwoodListRealmContactsOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}
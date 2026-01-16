// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodRemoveRealmContactAsync.h"

URedwoodRemoveRealmContactAsync *
URedwoodRemoveRealmContactAsync::RemoveRealmContact(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString OtherCharacterId
) {
  URedwoodRemoveRealmContactAsync *Action =
    NewObject<URedwoodRemoveRealmContactAsync>();
  Action->Target = Target;
  Action->OtherCharacterId = OtherCharacterId;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodRemoveRealmContactAsync::Activate() {
  Target->RemoveRealmContact(
    OtherCharacterId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Output) {
      OnOutput.Broadcast(Output);
      SetReadyToDestroy();
    })
  );
}
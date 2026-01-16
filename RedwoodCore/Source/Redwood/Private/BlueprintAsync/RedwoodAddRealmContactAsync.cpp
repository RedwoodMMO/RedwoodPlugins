// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodAddRealmContactAsync.h"

URedwoodAddRealmContactAsync *URedwoodAddRealmContactAsync::AddRealmContact(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString OtherCharacterId,
  bool bBlocked
) {
  URedwoodAddRealmContactAsync *Action =
    NewObject<URedwoodAddRealmContactAsync>();
  Action->Target = Target;
  Action->OtherCharacterId = OtherCharacterId;
  Action->bBlocked = bBlocked;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodAddRealmContactAsync::Activate() {
  Target->AddRealmContact(
    OtherCharacterId,
    bBlocked,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Output) {
      OnOutput.Broadcast(Output);
      SetReadyToDestroy();
    })
  );
}
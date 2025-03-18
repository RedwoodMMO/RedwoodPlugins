// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodSetCharacterArchivedAsync.h"

URedwoodSetCharacterArchivedAsync *
URedwoodSetCharacterArchivedAsync::SetCharacterArchived(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString CharacterId,
  bool bArchived
) {
  URedwoodSetCharacterArchivedAsync *Action =
    NewObject<URedwoodSetCharacterArchivedAsync>();
  Action->Target = Target;
  Action->CharacterId = CharacterId;
  Action->bArchived = bArchived;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodSetCharacterArchivedAsync::Activate() {
  Target->SetCharacterArchived(
    CharacterId,
    bArchived,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}

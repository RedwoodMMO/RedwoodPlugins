// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodCanAdminAllianceAsync.h"

URedwoodCanAdminAllianceAsync *URedwoodCanAdminAllianceAsync::CanAdminAlliance(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString AllianceId
) {
  URedwoodCanAdminAllianceAsync *Action =
    NewObject<URedwoodCanAdminAllianceAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->AllianceId = AllianceId;

  return Action;
}

void URedwoodCanAdminAllianceAsync::Activate() {
  Target->CanAdminAlliance(
    AllianceId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodUpdateAllianceAsync.h"

URedwoodUpdateAllianceAsync *URedwoodUpdateAllianceAsync::UpdateAlliance(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString AllianceId,
  FString AllianceName,
  bool bInviteOnly
) {
  URedwoodUpdateAllianceAsync *Action =
    NewObject<URedwoodUpdateAllianceAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->AllianceId = AllianceId;
  Action->AllianceName = AllianceName;
  Action->bInviteOnly = bInviteOnly;

  return Action;
}

void URedwoodUpdateAllianceAsync::Activate() {
  Target->UpdateAlliance(
    AllianceId,
    AllianceName,
    bInviteOnly,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
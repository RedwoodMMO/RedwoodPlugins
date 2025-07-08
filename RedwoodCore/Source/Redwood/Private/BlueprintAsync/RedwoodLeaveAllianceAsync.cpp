// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodLeaveAllianceAsync.h"

URedwoodLeaveAllianceAsync *URedwoodLeaveAllianceAsync::LeaveAlliance(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString AllianceId,
  FString GuildId
) {
  URedwoodLeaveAllianceAsync *Action = NewObject<URedwoodLeaveAllianceAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->AllianceId = AllianceId;
  Action->GuildId = GuildId;

  return Action;
}

void URedwoodLeaveAllianceAsync::Activate() {
  Target->LeaveAlliance(
    AllianceId,
    GuildId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
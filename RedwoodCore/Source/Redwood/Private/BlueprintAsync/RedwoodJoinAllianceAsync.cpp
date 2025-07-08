// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodJoinAllianceAsync.h"

URedwoodJoinAllianceAsync *URedwoodJoinAllianceAsync::JoinAlliance(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString AllianceId,
  FString GuildId
) {
  URedwoodJoinAllianceAsync *Action = NewObject<URedwoodJoinAllianceAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->AllianceId = AllianceId;
  Action->GuildId = GuildId;

  return Action;
}

void URedwoodJoinAllianceAsync::Activate() {
  Target->JoinAlliance(
    AllianceId,
    GuildId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
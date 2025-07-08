// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodKickGuildFromAllianceAsync.h"

URedwoodKickGuildFromAllianceAsync *
URedwoodKickGuildFromAllianceAsync::KickGuildFromAlliance(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString AllianceId,
  FString GuildId
) {
  URedwoodKickGuildFromAllianceAsync *Action =
    NewObject<URedwoodKickGuildFromAllianceAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->AllianceId = AllianceId;
  Action->GuildId = GuildId;

  return Action;
}

void URedwoodKickGuildFromAllianceAsync::Activate() {
  Target->KickGuildFromAlliance(
    AllianceId,
    GuildId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
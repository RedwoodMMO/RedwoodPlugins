// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodBanGuildFromAllianceAsync.h"

URedwoodBanGuildFromAllianceAsync *
URedwoodBanGuildFromAllianceAsync::BanGuildFromAlliance(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString AllianceId,
  FString GuildId
) {
  URedwoodBanGuildFromAllianceAsync *Action =
    NewObject<URedwoodBanGuildFromAllianceAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->AllianceId = AllianceId;
  Action->GuildId = GuildId;

  return Action;
}

void URedwoodBanGuildFromAllianceAsync::Activate() {
  Target->BanGuildFromAlliance(
    AllianceId,
    GuildId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
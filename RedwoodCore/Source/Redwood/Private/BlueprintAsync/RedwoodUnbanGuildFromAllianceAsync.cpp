// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodUnbanGuildFromAllianceAsync.h"

URedwoodUnbanGuildFromAllianceAsync *
URedwoodUnbanGuildFromAllianceAsync::UnbanGuildFromAlliance(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString GuildId,
  FString AllianceId
) {
  URedwoodUnbanGuildFromAllianceAsync *Action =
    NewObject<URedwoodUnbanGuildFromAllianceAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->GuildId = GuildId;
  Action->AllianceId = AllianceId;

  return Action;
}

void URedwoodUnbanGuildFromAllianceAsync::Activate() {
  Target->UnbanGuildFromAlliance(
    GuildId,
    AllianceId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
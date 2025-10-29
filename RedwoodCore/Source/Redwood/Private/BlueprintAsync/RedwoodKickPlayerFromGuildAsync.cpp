// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodKickPlayerFromGuildAsync.h"

URedwoodKickPlayerFromGuildAsync *
URedwoodKickPlayerFromGuildAsync::KickPlayerFromGuild(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString GuildId,
  FString TargetPlayerId
) {
  URedwoodKickPlayerFromGuildAsync *Action =
    NewObject<URedwoodKickPlayerFromGuildAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->GuildId = GuildId;
  Action->TargetPlayerId = TargetPlayerId;

  return Action;
}

void URedwoodKickPlayerFromGuildAsync::Activate() {
  Target->KickPlayerFromGuild(
    GuildId,
    TargetPlayerId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
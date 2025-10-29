// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodBanPlayerFromGuildAsync.h"

URedwoodBanPlayerFromGuildAsync *
URedwoodBanPlayerFromGuildAsync::BanPlayerFromGuild(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString GuildId,
  FString TargetPlayerId
) {
  URedwoodBanPlayerFromGuildAsync *Action =
    NewObject<URedwoodBanPlayerFromGuildAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->GuildId = GuildId;
  Action->TargetPlayerId = TargetPlayerId;

  return Action;
}

void URedwoodBanPlayerFromGuildAsync::Activate() {
  Target->BanPlayerFromGuild(
    GuildId,
    TargetPlayerId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
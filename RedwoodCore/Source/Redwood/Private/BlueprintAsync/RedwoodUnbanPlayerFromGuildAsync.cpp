// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodUnbanPlayerFromGuildAsync.h"

URedwoodUnbanPlayerFromGuildAsync *
URedwoodUnbanPlayerFromGuildAsync::UnbanPlayerFromGuild(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString GuildId,
  FString TargetPlayerId
) {
  URedwoodUnbanPlayerFromGuildAsync *Action =
    NewObject<URedwoodUnbanPlayerFromGuildAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->GuildId = GuildId;
  Action->TargetPlayerId = TargetPlayerId;

  return Action;
}

void URedwoodUnbanPlayerFromGuildAsync::Activate() {
  Target->UnbanPlayerFromGuild(
    GuildId,
    TargetPlayerId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodJoinGuildAsync.h"

URedwoodJoinGuildAsync *URedwoodJoinGuildAsync::JoinGuild(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString GuildId
) {
  URedwoodJoinGuildAsync *Action = NewObject<URedwoodJoinGuildAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->GuildId = GuildId;

  return Action;
}

void URedwoodJoinGuildAsync::Activate() {
  Target->JoinGuild(
    GuildId, FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
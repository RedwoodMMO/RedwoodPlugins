// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodInviteToGuildAsync.h"

URedwoodInviteToGuildAsync *URedwoodInviteToGuildAsync::InviteToGuild(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString GuildId,
  FString TargetPlayerId
) {
  URedwoodInviteToGuildAsync *Action = NewObject<URedwoodInviteToGuildAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->GuildId = GuildId;
  Action->TargetPlayerId = TargetPlayerId;

  return Action;
}

void URedwoodInviteToGuildAsync::Activate() {
  Target->InviteToGuild(
    GuildId,
    TargetPlayerId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
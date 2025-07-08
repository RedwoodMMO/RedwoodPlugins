// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodUpdateGuildAsync.h"

URedwoodUpdateGuildAsync *URedwoodUpdateGuildAsync::UpdateGuild(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString GuildId,
  FString GuildName,
  ERedwoodGuildInviteType InviteType,
  bool bListed,
  bool bMembershipPublic
) {
  URedwoodUpdateGuildAsync *Action = NewObject<URedwoodUpdateGuildAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->GuildId = GuildId;
  Action->GuildName = GuildName;
  Action->InviteType = InviteType;
  Action->bListed = bListed;
  Action->bMembershipPublic = bMembershipPublic;

  return Action;
}

void URedwoodUpdateGuildAsync::Activate() {
  Target->UpdateGuild(
    GuildId,
    GuildName,
    InviteType,
    bListed,
    bMembershipPublic,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodCreateGuildAsync.h"

URedwoodCreateGuildAsync *URedwoodCreateGuildAsync::CreateGuild(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString GuildName,
  FString GuildTag,
  ERedwoodGuildInviteType InviteType,
  bool bListed,
  bool bMembershipPublic
) {
  URedwoodCreateGuildAsync *Action = NewObject<URedwoodCreateGuildAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->GuildName = GuildName;
  Action->GuildTag = GuildTag;
  Action->InviteType = InviteType;
  Action->bListed = bListed;
  Action->bMembershipPublic = bMembershipPublic;

  return Action;
}

void URedwoodCreateGuildAsync::Activate() {
  Target->CreateGuild(
    GuildName,
    GuildTag,
    InviteType,
    bListed,
    bMembershipPublic,
    FRedwoodCreateGuildOutputDelegate::CreateLambda(
      [this](FRedwoodCreateGuildOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}
// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodCreateGuildAsync.h"

URedwoodCreateGuildAsync *URedwoodCreateGuildAsync::CreateGuild(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString GuildName,
  ERedwoodGuildInviteType InviteType,
  bool bListed,
  bool bMembershipPublic
) {
  URedwoodCreateGuildAsync *Action = NewObject<URedwoodCreateGuildAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->GuildName = GuildName;
  Action->InviteType = InviteType;
  Action->bListed = bListed;
  Action->bMembershipPublic = bMembershipPublic;

  return Action;
}

void URedwoodCreateGuildAsync::Activate() {
  Target->CreateGuild(
    GuildName,
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
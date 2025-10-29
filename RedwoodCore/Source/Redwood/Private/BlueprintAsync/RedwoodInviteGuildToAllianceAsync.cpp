// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodInviteGuildToAllianceAsync.h"

URedwoodInviteGuildToAllianceAsync *
URedwoodInviteGuildToAllianceAsync::InviteGuildToAlliance(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString AllianceId,
  FString GuildId
) {
  URedwoodInviteGuildToAllianceAsync *Action =
    NewObject<URedwoodInviteGuildToAllianceAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->AllianceId = AllianceId;
  Action->GuildId = GuildId;

  return Action;
}

void URedwoodInviteGuildToAllianceAsync::Activate() {
  Target->InviteGuildToAlliance(
    AllianceId,
    GuildId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
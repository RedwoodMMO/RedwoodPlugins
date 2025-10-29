// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodCreateAllianceAsync.h"

URedwoodCreateAllianceAsync *URedwoodCreateAllianceAsync::CreateAlliance(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString AllianceName,
  FString GuildId,
  bool bInviteOnly
) {
  URedwoodCreateAllianceAsync *Action =
    NewObject<URedwoodCreateAllianceAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->AllianceName = AllianceName;
  Action->GuildId = GuildId;
  Action->bInviteOnly = bInviteOnly;

  return Action;
}

void URedwoodCreateAllianceAsync::Activate() {
  Target->CreateAlliance(
    AllianceName,
    GuildId,
    bInviteOnly,
    FRedwoodCreateAllianceOutputDelegate::CreateLambda(
      [this](FRedwoodCreateAllianceOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}
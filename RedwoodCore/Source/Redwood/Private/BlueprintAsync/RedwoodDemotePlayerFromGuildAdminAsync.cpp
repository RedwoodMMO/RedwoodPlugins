// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodDemotePlayerFromGuildAdminAsync.h"

URedwoodDemotePlayerFromGuildAdminAsync *
URedwoodDemotePlayerFromGuildAdminAsync::DemotePlayerFromGuildAdmin(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString GuildId,
  FString TargetPlayerId
) {
  URedwoodDemotePlayerFromGuildAdminAsync *Action =
    NewObject<URedwoodDemotePlayerFromGuildAdminAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->GuildId = GuildId;
  Action->TargetPlayerId = TargetPlayerId;

  return Action;
}

void URedwoodDemotePlayerFromGuildAdminAsync::Activate() {
  Target->DemotePlayerFromGuildAdmin(
    GuildId,
    TargetPlayerId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
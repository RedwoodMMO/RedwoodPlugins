// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodPromotePlayerToGuildAdminAsync.h"

URedwoodPromotePlayerToGuildAdminAsync *
URedwoodPromotePlayerToGuildAdminAsync::PromotePlayerToGuildAdmin(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString GuildId,
  FString TargetPlayerId
) {
  URedwoodPromotePlayerToGuildAdminAsync *Action =
    NewObject<URedwoodPromotePlayerToGuildAdminAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->GuildId = GuildId;
  Action->TargetPlayerId = TargetPlayerId;

  return Action;
}

void URedwoodPromotePlayerToGuildAdminAsync::Activate() {
  Target->PromotePlayerToGuildAdmin(
    GuildId,
    TargetPlayerId,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
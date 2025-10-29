// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodSetSelectedGuildAsync.h"

URedwoodSetSelectedGuildAsync *URedwoodSetSelectedGuildAsync::SetSelectedGuild(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString GuildId
) {
  URedwoodSetSelectedGuildAsync *Action =
    NewObject<URedwoodSetSelectedGuildAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->GuildId = GuildId;

  return Action;
}

void URedwoodSetSelectedGuildAsync::Activate() {
  Target->SetSelectedGuild(
    GuildId, FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
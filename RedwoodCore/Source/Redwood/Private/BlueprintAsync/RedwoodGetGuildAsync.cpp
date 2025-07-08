// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodGetGuildAsync.h"

URedwoodGetGuildAsync *URedwoodGetGuildAsync::GetGuild(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString GuildId
) {
  URedwoodGetGuildAsync *Action = NewObject<URedwoodGetGuildAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->GuildId = GuildId;

  return Action;
}

void URedwoodGetGuildAsync::Activate() {
  Target->GetGuild(
    GuildId,
    FRedwoodGetGuildOutputDelegate::CreateLambda(
      [this](FRedwoodGetGuildOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}
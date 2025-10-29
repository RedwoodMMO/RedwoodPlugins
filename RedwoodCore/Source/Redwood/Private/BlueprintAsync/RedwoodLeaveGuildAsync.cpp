// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodLeaveGuildAsync.h"

URedwoodLeaveGuildAsync *URedwoodLeaveGuildAsync::LeaveGuild(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString GuildId
) {
  URedwoodLeaveGuildAsync *Action = NewObject<URedwoodLeaveGuildAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->GuildId = GuildId;

  return Action;
}

void URedwoodLeaveGuildAsync::Activate() {
  Target->LeaveGuild(
    GuildId, FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
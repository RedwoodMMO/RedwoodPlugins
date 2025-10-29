// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodListGuildMembersAsync.h"

URedwoodListGuildMembersAsync *URedwoodListGuildMembersAsync::ListGuildMembers(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  FString GuildId,
  ERedwoodGuildAndAllianceMemberState State
) {
  URedwoodListGuildMembersAsync *Action =
    NewObject<URedwoodListGuildMembersAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);
  Action->GuildId = GuildId;
  Action->State = State;

  return Action;
}

void URedwoodListGuildMembersAsync::Activate() {
  Target->ListGuildMembers(
    GuildId,
    State,
    FRedwoodListGuildMembersOutputDelegate::CreateLambda(
      [this](FRedwoodListGuildMembersOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}
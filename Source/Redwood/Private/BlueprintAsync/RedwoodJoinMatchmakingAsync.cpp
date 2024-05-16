// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodJoinMatchmakingAsync.h"

URedwoodJoinMatchmakingAsync *URedwoodJoinMatchmakingAsync::JoinMatchmaking(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  FString ProfileId,
  TArray<FString> Regions
) {
  URedwoodJoinMatchmakingAsync *Action =
    NewObject<URedwoodJoinMatchmakingAsync>();
  Action->Target = Target;
  Action->ProfileId = ProfileId;
  Action->Regions = Regions;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodJoinMatchmakingAsync::Activate() {
  Target->JoinMatchmaking(
    ProfileId,
    Regions,
    FRedwoodTicketingUpdateDelegate::CreateLambda(
      [this](FRedwoodTicketingUpdate Update) {
        OnUpdate.Broadcast(Update);

        if (
          Update.Type == ERedwoodTicketingUpdateType::TicketStale ||
          (
            Update.Type == ERedwoodTicketingUpdateType::JoinResponse &&
            !Update.Message.IsEmpty()
          )
        ) {
          SetReadyToDestroy();
        }
      }
    )
  );
}

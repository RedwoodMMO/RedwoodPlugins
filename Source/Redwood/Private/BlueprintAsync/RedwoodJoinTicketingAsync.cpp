// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodJoinTicketingAsync.h"

URedwoodJoinTicketingAsync *URedwoodJoinTicketingAsync::JoinTicketing(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  TArray<FString> ModeIds,
  TArray<FString> Regions
) {
  URedwoodJoinTicketingAsync *Action = NewObject<URedwoodJoinTicketingAsync>();
  Action->Target = Target;
  Action->ModeIds = ModeIds;
  Action->Regions = Regions;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodJoinTicketingAsync::Activate() {
  Target->JoinTicketing(
    ModeIds,
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

// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodJoinTicketingAsync.h"

URedwoodJoinTicketingAsync *URedwoodJoinTicketingAsync::JoinTicketing(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  FString Profile
) {
  URedwoodJoinTicketingAsync *Action = NewObject<URedwoodJoinTicketingAsync>();
  Action->Target = Target;
  Action->Profile = Profile;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodJoinTicketingAsync::Activate() {
  Target->JoinTicketing(
    Profile,
    URedwoodTitleGameSubsystem::FRedwoodOnTicketingUpdate::CreateLambda(
      [this](FRedwoodTicketingUpdate Update) {
        OnUpdate.Broadcast(Update);

        if (
          Update.Type == ERedwoodTicketingUpdateType::Ready ||
          Update.Type == ERedwoodTicketingUpdateType::TicketStale ||
          (
            Update.Type == ERedwoodTicketingUpdateType::JoinResponse &&
            Update.Message != TEXT("")
          )
        ) {
          SetReadyToDestroy();
        }
      }
    )
  );
}

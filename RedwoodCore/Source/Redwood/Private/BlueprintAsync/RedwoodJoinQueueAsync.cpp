// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodJoinQueueAsync.h"

URedwoodJoinQueueAsync *URedwoodJoinQueueAsync::JoinQueue(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  FString ProxyId,
  FString ZoneName
) {
  URedwoodJoinQueueAsync *Action = NewObject<URedwoodJoinQueueAsync>();
  Action->Target = Target;
  Action->ProxyId = ProxyId;
  Action->ZoneName = ZoneName;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodJoinQueueAsync::Activate() {
  Target->JoinQueue(
    ProxyId,
    ZoneName,
    FRedwoodTicketingUpdateDelegate::CreateLambda(
      [this](FRedwoodTicketingUpdate Update) {
        OnUpdate.Broadcast(Update);

        if (
          Update.Type == ERedwoodTicketingUpdateType::TicketError ||
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

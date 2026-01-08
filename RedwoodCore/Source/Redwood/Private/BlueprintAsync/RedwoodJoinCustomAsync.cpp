// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodJoinCustomAsync.h"

URedwoodJoinCustomAsync *URedwoodJoinCustomAsync::JoinCustom(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  bool bTransferWholeParty,
  TArray<FString> Regions
) {
  URedwoodJoinCustomAsync *Action = NewObject<URedwoodJoinCustomAsync>();
  Action->Target = Target;
  Action->bTransferWholeParty = bTransferWholeParty;
  Action->Regions = Regions;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodJoinCustomAsync::Activate() {
  Target->JoinCustom(
    bTransferWholeParty,
    Regions,
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

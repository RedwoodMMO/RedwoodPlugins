// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodCreateCustomRoomAsync.h"

URedwoodCreateCustomRoomAsync *URedwoodCreateCustomRoomAsync::CreateCustomRoom(
  URedwoodClientChatSubsystem *Target,
  UObject *WorldContextObject,
  FString Id,
  FString Password
) {
  URedwoodCreateCustomRoomAsync *Action =
    NewObject<URedwoodCreateCustomRoomAsync>();
  Action->Target = Target;
  Action->Id = Id;
  Action->Password = Password;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodCreateCustomRoomAsync::Activate() {
  Target->CreateCustomRoom(
    Id,
    Password,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
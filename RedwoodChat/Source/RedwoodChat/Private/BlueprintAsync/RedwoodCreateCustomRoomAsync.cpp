// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodCreateCustomRoomAsync.h"

URedwoodCreateCustomRoomAsync *URedwoodCreateCustomRoomAsync::CreateCustomRoom(
  URedwoodClientChatSubsystem *Target,
  UObject *WorldContextObject,
  FString Id,
  FString Password,
  bool bCreateAsCharacter
) {
  URedwoodCreateCustomRoomAsync *Action =
    NewObject<URedwoodCreateCustomRoomAsync>();
  Action->Target = Target;
  Action->Id = Id;
  Action->Password = Password;
  Action->bCreateAsCharacter = bCreateAsCharacter;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodCreateCustomRoomAsync::Activate() {
  Target->CreateCustomRoom(
    Id,
    Password,
    bCreateAsCharacter,
    FRedwoodChatRoomCreatedOutputDelegate::CreateLambda(
      [this](const FString &Error, const FString &JoinCode) {
        OnOutput.Broadcast(Error, JoinCode);
        SetReadyToDestroy();
      }
    )
  );
}

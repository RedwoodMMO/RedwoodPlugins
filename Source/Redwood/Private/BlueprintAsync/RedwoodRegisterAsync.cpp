// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodRegisterAsync.h"

URedwoodRegisterAsync *URedwoodRegisterAsync::Register(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  const FString &Username,
  const FString &Password
) {
  URedwoodRegisterAsync *Action = NewObject<URedwoodRegisterAsync>();
  Action->Target = Target;
  Action->Username = Username;
  Action->Password = Password;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodRegisterAsync::Activate() {
  Target->Register(
    Username,
    Password,
    FRedwoodAuthUpdateDelegate::CreateLambda([this](FRedwoodAuthUpdate Update) {
      OnUpdate.Broadcast(Update);

      if (Update.Type != ERedwoodAuthUpdateType::MustVerifyAccount) {
        SetReadyToDestroy();
      }
    })
  );
}

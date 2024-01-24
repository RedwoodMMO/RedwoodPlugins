// Copyright Incanta Games. All Rights Reserved.

#include "LatentActions/RedwoodLoginAsync.h"

URedwoodLoginAsync *URedwoodLoginAsync::Login(
  URedwoodTitleGameSubsystem *Target,
  UObject *WorldContextObject,
  const FString &Username,
  const FString &Password
) {
  URedwoodLoginAsync *Action = NewObject<URedwoodLoginAsync>();
  Action->Target = Target;
  Action->Username = Username;
  Action->Password = Password;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodLoginAsync::Activate() {
  Target->Login(
    Username,
    Password,
    URedwoodTitleGameSubsystem::FRedwoodOnAuthUpdate::CreateLambda(
      [this](FRedwoodAuthUpdate Update) {
        OnUpdate.Broadcast(Update);

        if (Update.Type != ERedwoodAuthUpdateType::MustVerifyAccount) {
          SetReadyToDestroy();
        }
      }
    )
  );
}

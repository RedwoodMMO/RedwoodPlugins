// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodLoginWithSteamAsync.h"

URedwoodLoginWithSteamAsync *URedwoodLoginWithSteamAsync::LoginWithSteam(
  URedwoodTitleGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodLoginWithSteamAsync *Action =
    NewObject<URedwoodLoginWithSteamAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodLoginWithSteamAsync::Activate() {
  URedwoodSteamTitleInterface::LoginWithSteam(
    Target->GetTitleInterface(),
    FRedwoodAuthUpdateDelegate::CreateLambda([this](FRedwoodAuthUpdate Update) {
      OnUpdate.Broadcast(Update);

      if (Update.Type != ERedwoodAuthUpdateType::MustVerifyAccount) {
        SetReadyToDestroy();
      }
    })
  );
}

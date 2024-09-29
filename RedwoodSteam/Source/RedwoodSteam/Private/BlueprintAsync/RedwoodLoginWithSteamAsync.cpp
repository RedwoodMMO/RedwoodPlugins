// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodLoginWithSteamAsync.h"

#include "RedwoodSteamClientInterface.h"

URedwoodLoginWithSteamAsync *URedwoodLoginWithSteamAsync::LoginWithSteam(
  URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodLoginWithSteamAsync *Action =
    NewObject<URedwoodLoginWithSteamAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodLoginWithSteamAsync::Activate() {
  URedwoodSteamClientInterface::LoginWithSteam(
    Target->GetClientInterface(),
    FRedwoodAuthUpdateDelegate::CreateLambda([this](FRedwoodAuthUpdate Update) {
      OnUpdate.Broadcast(Update);

      if (Update.Type != ERedwoodAuthUpdateType::MustVerifyAccount) {
        SetReadyToDestroy();
      }
    })
  );
}

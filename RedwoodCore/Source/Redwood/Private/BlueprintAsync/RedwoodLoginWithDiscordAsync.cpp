// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodLoginWithDiscordAsync.h"

URedwoodLoginWithDiscordAsync *URedwoodLoginWithDiscordAsync::LoginWithDiscord(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  bool bRememberMe
) {
  URedwoodLoginWithDiscordAsync *Action =
    NewObject<URedwoodLoginWithDiscordAsync>();
  Action->Target = Target;
  Action->bRememberMe = bRememberMe;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodLoginWithDiscordAsync::Activate() {
  Target->LoginWithDiscord(
    bRememberMe,
    FRedwoodAuthUpdateDelegate::CreateLambda([this](FRedwoodAuthUpdate Update) {
      OnUpdate.Broadcast(Update);

      SetReadyToDestroy();
    })
  );
}

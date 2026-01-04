// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodLoginWithTwitchAsync.h"

URedwoodLoginWithTwitchAsync *URedwoodLoginWithTwitchAsync::LoginWithTwitch(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  bool bRememberMe
) {
  URedwoodLoginWithTwitchAsync *Action =
    NewObject<URedwoodLoginWithTwitchAsync>();
  Action->Target = Target;
  Action->bRememberMe = bRememberMe;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodLoginWithTwitchAsync::Activate() {
  Target->LoginWithTwitch(
    bRememberMe,
    FRedwoodAuthUpdateDelegate::CreateLambda([this](FRedwoodAuthUpdate Update) {
      OnUpdate.Broadcast(Update);

      SetReadyToDestroy();
    })
  );
}

// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodLoginWithEOSAsync.h"

#include "RedwoodEOSClientInterface.h"

URedwoodLoginWithEOSAsync *URedwoodLoginWithEOSAsync::LoginWithEOS(
  URedwoodClientGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodLoginWithEOSAsync *Action = NewObject<URedwoodLoginWithEOSAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodLoginWithEOSAsync::Activate() {
  URedwoodEOSClientInterface::LoginWithEOS(
    Target->GetClientInterface(),
    FRedwoodAuthUpdateDelegate::CreateLambda([this](FRedwoodAuthUpdate Update) {
      OnUpdate.Broadcast(Update);

      if (Update.Type != ERedwoodAuthUpdateType::MustVerifyAccount) {
        SetReadyToDestroy();
      }
    })
  );
}

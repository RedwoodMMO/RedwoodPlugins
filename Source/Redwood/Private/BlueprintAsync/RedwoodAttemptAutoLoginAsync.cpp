// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodAttemptAutoLoginAsync.h"

URedwoodAttemptAutoLoginAsync *URedwoodAttemptAutoLoginAsync::AttemptAutoLogin(
  URedwoodTitleGameSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodAttemptAutoLoginAsync *Action =
    NewObject<URedwoodAttemptAutoLoginAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodAttemptAutoLoginAsync::Activate() {
  Target->AttemptAutoLogin(
    FRedwoodAuthUpdateDelegate::CreateLambda([this](FRedwoodAuthUpdate Update) {
      OnUpdate.Broadcast(Update);

      if (Update.Type != ERedwoodAuthUpdateType::MustVerifyAccount) {
        SetReadyToDestroy();
      }
    })
  );
}

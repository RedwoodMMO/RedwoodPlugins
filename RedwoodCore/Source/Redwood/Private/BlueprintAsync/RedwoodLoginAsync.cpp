// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodLoginAsync.h"

URedwoodLoginAsync *URedwoodLoginAsync::Login(
  URedwoodClientGameSubsystem *Target,
  UObject *WorldContextObject,
  const FString &Username,
  const FString &Password,
  bool bRememberMe,
  const FString &Provider
) {
  URedwoodLoginAsync *Action = NewObject<URedwoodLoginAsync>();
  Action->Target = Target;
  Action->Username = Username;
  Action->Password = Password;
  Action->bRememberMe = bRememberMe;
  Action->Provider = Provider;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodLoginAsync::Activate() {
  Target->Login(
    Username,
    Password,
    Provider,
    bRememberMe,
    FRedwoodAuthUpdateDelegate::CreateLambda([this](FRedwoodAuthUpdate Update) {
      OnUpdate.Broadcast(Update);

      if (Update.Type != ERedwoodAuthUpdateType::MustVerifyAccount) {
        SetReadyToDestroy();
      }
    })
  );
}

// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodInitializeChatConnectionAsync.h"

URedwoodInitializeChatConnectionAsync *
URedwoodInitializeChatConnectionAsync::InitializeChatConnection(
  URedwoodChatClientSubsystem *Target, UObject *WorldContextObject
) {
  URedwoodInitializeChatConnectionAsync *Action =
    NewObject<URedwoodInitializeChatConnectionAsync>();
  Action->Target = Target;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodInitializeChatConnectionAsync::Activate() {
  Target->InitializeChatConnection(
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}
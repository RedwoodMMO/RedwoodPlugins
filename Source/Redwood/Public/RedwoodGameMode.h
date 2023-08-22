// Copyright Incanta Games 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "SocketIONative.h"

#include "RedwoodGameMode.generated.h"

UCLASS()
class REDWOOD_API ARedwoodGameMode : public AGameMode {
  GENERATED_BODY()

public:

  //~AGameModeBase interface
  virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
  virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
  //~End of AGameModeBase interface

private:

  TMap<FString, APlayerController*> PendingAuthenticationRequests;

  TSharedPtr<FSocketIONative> Sidecar;
};

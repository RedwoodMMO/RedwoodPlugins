// Copyright Incanta Games 2023. All Rights Reserved.

#pragma once

#include "RedwoodModule.h"
#include "RedwoodGameplayTags.h"

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/TimerHandle.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "SocketIOFunctionLibrary.h"
#include "SocketIONative.h"

#include "RedwoodGameSubsystem.generated.h"

UCLASS()
class URedwoodGameSubsystem : public UGameInstanceSubsystem {
  GENERATED_BODY()

public:

  // Begin USubsystem
  virtual void Initialize(FSubsystemCollectionBase& Collection) override;
  virtual void Deinitialize() override;
  // End USubsystem

private:

  void InitializeSidecar();
  void SendUpdateToSidecar();

  TSharedPtr<FSocketIONative> Sidecar;

  float UpdateSidecarRate = 3.f; // in seconds
  float UpdateSidecarLoadingRate = 0.2f; // in seconds
  FTimerHandle TimerHandle_UpdateSidecar;
  FTimerHandle TimerHandle_UpdateSidecarLoading;

  bool bIsShuttingDown = false;
	FGameplayMessageListenerHandle ListenerHandle;
	void OnShutdownMessage(FGameplayTag Channel, const FRedwoodReason& Message);
};

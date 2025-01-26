// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodSyncComponent.h"
#include "RedwoodServerGameSubsystem.h"
#include "TimerManager.h"

void URedwoodSyncComponent::BeginPlay() {
  Super::BeginPlay();

  if (GetWorld()->GetNetMode() == NM_DedicatedServer) {
    GetWorld()->GetTimerManager().SetTimerForNextTick(
      this, &URedwoodSyncComponent::InitSyncComponent
    );
  }
}

void URedwoodSyncComponent::InitSyncComponent() {
  URedwoodServerGameSubsystem *Subsystem =
    GetWorld()->GetGameInstance()->GetSubsystem<URedwoodServerGameSubsystem>();

  if (Subsystem) {
    if (ZoneName == TEXT("")) {
      // This should only happen on servers that initiated the spawning.
      // Other servers should set this before Begin Play is fired during
      // sync creation.
      ZoneName = Subsystem->ZoneName;

      Subsystem->RegisterSyncComponent(this);

      InitiallySpawned.Broadcast();
    }
  }
}

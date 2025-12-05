// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodSyncComponent.h"
#include "RedwoodModule.h"
#include "RedwoodServerGameSubsystem.h"

#include "GameFramework/GameStateBase.h"
#include "TimerManager.h"

void URedwoodSyncComponent::BeginPlay() {
  Super::BeginPlay();

  if (GetWorld()->GetNetMode() == NM_DedicatedServer) {
    if (Cast<AGameStateBase>(GetOwner()) && !bPersistChanges) {
      UE_LOG(
        LogRedwood,
        Warning,
        TEXT(
          "URedwoodSyncComponent '%s' is attached to a class that inherits from AGameStateBase ('%s') but bPersistChanges is false. Usually sync components in the GameState are meant to persist (e.g. proxy and zone data)."
        ),
        *GetName(),
        *GetOwner()->GetName()
      );
    }

    GetWorld()->GetTimerManager().SetTimerForNextTick(
      this, &URedwoodSyncComponent::InitSyncComponent
    );
  }
}

void URedwoodSyncComponent::InitSyncComponent() {
  URedwoodServerGameSubsystem *Subsystem =
    GetWorld()->GetGameInstance()->GetSubsystem<URedwoodServerGameSubsystem>();

  if (Subsystem) {
    if (ZoneName == TEXT("") || ZoneName == Subsystem->ZoneName) {
      // This should only happen on servers that initiated the spawning;
      // it can also happen for items placed in the editor that spawn on
      // level load. Other servers should set this before Begin Play is
      // fired during sync creation.

      bool bDelayNewSync = ZoneName == Subsystem->ZoneName;

      ZoneName = Subsystem->ZoneName;

      Subsystem->RegisterSyncComponent(this, bDelayNewSync);

      InitiallySpawned.Broadcast();
    }
  }
}

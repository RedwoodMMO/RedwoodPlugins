// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodSyncComponent.h"
#include "RedwoodServerGameSubsystem.h"

void URedwoodSyncComponent::BeginPlay() {
  Super::BeginPlay();

  if (GetWorld()->GetNetMode() == NM_DedicatedServer) {
    URedwoodServerGameSubsystem *Subsystem =
      GetWorld()->GetGameInstance()->GetSubsystem<URedwoodServerGameSubsystem>(
      );

    if (Subsystem) {
      Subsystem->RegisterSyncComponent(this);
    }
  }
}

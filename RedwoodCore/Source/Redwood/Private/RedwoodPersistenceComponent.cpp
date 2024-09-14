// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodPersistenceComponent.h"
#include "RedwoodServerGameSubsystem.h"

void URedwoodPersistenceComponent::BeginPlay() {
  Super::BeginPlay();

  if (GetWorld()->GetNetMode() == NM_DedicatedServer) {
    URedwoodServerGameSubsystem *Subsystem =
      GetWorld()->GetGameInstance()->GetSubsystem<URedwoodServerGameSubsystem>(
      );

    if (Subsystem) {
      Subsystem->RegisterPersistenceComponent(this);
    }
  }
}

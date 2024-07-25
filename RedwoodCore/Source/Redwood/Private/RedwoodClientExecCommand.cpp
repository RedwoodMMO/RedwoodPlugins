// Copyright Incanta Games. All rights reserved.

#include "RedwoodClientExecCommand.h"
#include "Net/UnrealNetwork.h"

void ARedwoodClientExecCommand::BeginPlay() {
  Super::BeginPlay();

  UWorld *World = GetWorld();

  if (IsValid(World) && World->GetNetMode() == ENetMode::NM_Client && !Command.IsEmpty()) {
    GetWorld()->Exec(World, *Command);
  }
}

void ARedwoodClientExecCommand::GetLifetimeReplicatedProps(
  TArray<FLifetimeProperty> &OutLifetimeProps
) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(ARedwoodClientExecCommand, Command);
}

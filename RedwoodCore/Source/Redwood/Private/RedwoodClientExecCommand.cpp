// Copyright Incanta Games. All rights reserved.

#include "RedwoodClientExecCommand.h"
#include "Net/Core/PushModel/PushModel.h"
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

  // Command is set once at deferred spawn (before FinishSpawningActor), so its
  // value rides the initial replication; no MARK_PROPERTY_DIRTY is needed because
  // it never mutates after the actor begins replicating.
  FDoRepLifetimeParams Params;
  Params.bIsPushBased = true;
  DOREPLIFETIME_WITH_PARAMS_FAST(ARedwoodClientExecCommand, Command, Params);
}

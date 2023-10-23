// Copyright Incanta Games 2023. All rights reserved.

#include "RedwoodTitleGameModeBase.h"
#include "RedwoodTitlePlayerController.h"

ARedwoodTitleGameModeBase::ARedwoodTitleGameModeBase(
  const FObjectInitializer &ObjectInitializer
) :
  Super(ObjectInitializer) {
  PlayerControllerClass = ARedwoodTitlePlayerController::StaticClass();
}

// Copyright Incanta LLC. All rights reserved.

#include "RedwoodPlayerState.h"

void ARedwoodPlayerState::SetClientReady_Implementation() {
  if (GetLocalRole() == ROLE_Authority) {
    bClientReady = true;
  }
}

bool ARedwoodPlayerState::SetClientReady_Validate() {
  return true;
}

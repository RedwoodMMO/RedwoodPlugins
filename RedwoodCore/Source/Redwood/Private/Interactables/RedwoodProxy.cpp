// Copyright Incanta Games. All Rights Reserved.

#include "Interactables/RedwoodProxy.h"

ARedwoodProxy::ARedwoodProxy() {
  bReplicates = true;
  bNetUseOwnerRelevancy = false;
  bOnlyRelevantToOwner = true;
}

void ARedwoodProxy::GetLifetimeReplicatedProps(
  TArray<FLifetimeProperty> &OutLifetimeProps
) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(ARedwoodProxy, Interactable);
}

// Copyright Incanta Games. All Rights Reserved.

#include "Interactables/RedwoodProxy.h"

#include "Net/Core/PushModel/PushModel.h"
#include "Net/UnrealNetwork.h"

ARedwoodProxy::ARedwoodProxy() {
  bReplicates = true;
  bNetUseOwnerRelevancy = false;
  bOnlyRelevantToOwner = true;
}

void ARedwoodProxy::GetLifetimeReplicatedProps(
  TArray<FLifetimeProperty> &OutLifetimeProps
) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

#if WITH_PUSH_MODEL
  if (IS_PUSH_MODEL_ENABLED()) {
    FDoRepLifetimeParams Params;
    Params.bIsPushBased = true;
    DOREPLIFETIME_WITH_PARAMS_FAST(ARedwoodProxy, Interactable, Params);
  } else
#endif
  {
    DOREPLIFETIME(ARedwoodProxy, Interactable);
  }
}

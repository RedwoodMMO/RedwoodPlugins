// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RedwoodProxy.generated.h"

class ARedwoodInteractableProxied;

UCLASS(BlueprintType, Blueprintable)
class REDWOOD_API ARedwoodProxy : public AActor {
  GENERATED_BODY()

public:
  ARedwoodProxy();

  UPROPERTY(BlueprintReadOnly, Replicated, Category = "Redwood")
  ARedwoodInteractableProxied *Interactable;

protected:
  virtual void GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps
  ) const override;
};

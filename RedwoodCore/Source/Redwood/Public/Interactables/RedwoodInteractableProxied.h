// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RedwoodInteractable.h"
#include "RedwoodProxy.h"

#include "RedwoodInteractableProxied.generated.h"

class URedwoodCharacterComponent;

UCLASS(BlueprintType, Blueprintable)
class REDWOOD_API ARedwoodInteractableProxied : public ARedwoodInteractable {
  GENERATED_BODY()

public:
  virtual void OnInteract_Implementation(
    APawn *Pawn, URedwoodCharacterComponent *CharacterComponent
  ) override;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  TSubclassOf<ARedwoodProxy> ProxyClass;
};

// Copyright Incanta Games. All Rights Reserved.

#include "Interactables/RedwoodInteractableProxied.h"
#include "RedwoodCharacter.h"

void ARedwoodInteractableProxied::OnInteract_Implementation(
  ARedwoodCharacter *Character
) {
  if (IsValid(ProxyClass)) {
    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = this;
    SpawnParameters.Instigator = Character;

    AActor *Proxy = GetWorld()->SpawnActor<AActor>(
      ProxyClass, GetActorLocation(), GetActorRotation(), SpawnParameters
    );
    if (IsValid(Proxy)) {
      Proxy->SetOwner(Character);

      ARedwoodProxy *RedwoodProxy = Cast<ARedwoodProxy>(Proxy);
      if (IsValid(RedwoodProxy)) {
        RedwoodProxy->Interactable = this;
      }
    }
  }
}

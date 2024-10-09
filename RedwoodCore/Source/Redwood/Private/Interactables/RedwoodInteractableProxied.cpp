// Copyright Incanta Games. All Rights Reserved.

#include "Interactables/RedwoodInteractableProxied.h"
#include "RedwoodCharacterComponent.h"

void ARedwoodInteractableProxied::OnInteract_Implementation(
  APawn *Pawn, URedwoodCharacterComponent *CharacterComponent
) {
  if (IsValid(ProxyClass)) {
    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = this;
    SpawnParameters.Instigator = Pawn;

    AActor *Proxy = GetWorld()->SpawnActor<AActor>(
      ProxyClass, GetActorLocation(), GetActorRotation(), SpawnParameters
    );
    if (IsValid(Proxy)) {
      Proxy->SetOwner(Pawn);

      ARedwoodProxy *RedwoodProxy = Cast<ARedwoodProxy>(Proxy);
      if (IsValid(RedwoodProxy)) {
        RedwoodProxy->Interactable = this;
      }
    }
  }
}

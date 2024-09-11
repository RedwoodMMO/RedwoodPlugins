// Copyright Incanta Games. All Rights Reserved.

#include "Interactables/RedwoodInteractComponent.h"
#include "Interactables/RedwoodInteractable.h"
#include "RedwoodCharacter.h"
#include "RedwoodModule.h"

#include "Components/CapsuleComponent.h"

URedwoodInteractComponent::URedwoodInteractComponent() {
  SetIsReplicatedByDefault(true);
}

void URedwoodInteractComponent::BeginPlay() {
  Super::BeginPlay();

  ARedwoodCharacter *Character = Cast<ARedwoodCharacter>(GetOwner());
  if (!IsValid(Character)) {
    return;
  }

  UCapsuleComponent *CapsuleComponent = Character->GetCapsuleComponent();
  if (!IsValid(CapsuleComponent)) {
    return;
  }

  if (!Character->HasAuthority()) {
    CapsuleComponent->OnComponentBeginOverlap.AddDynamic(
      this, &URedwoodInteractComponent::OnCapsuleComponentBeginOverlap
    );

    CapsuleComponent->OnComponentEndOverlap.AddDynamic(
      this, &URedwoodInteractComponent::OnCapsuleComponentEndOverlap
    );
  }
}

void URedwoodInteractComponent::OnInteractionAvailability_Implementation(
  bool bAvailable
) {}

void URedwoodInteractComponent::OnCapsuleComponentBeginOverlap(
  UPrimitiveComponent *OverlappedComponent,
  AActor *OtherActor,
  UPrimitiveComponent *OtherComp,
  int32 OtherBodyIndex,
  bool bFromSweep,
  const FHitResult &SweepResult
) {
  ARedwoodInteractable *Interactable = Cast<ARedwoodInteractable>(OtherActor);
  if (IsValid(Interactable) || CanInteract()) {
    if (IsValid(Interactable) && Interactable->bAutoInteract) {
      RPC_Interact();
    } else {
      bReportedInteractionAvailability = true;
      OnInteractionAvailability(true);
    }
  }
}

void URedwoodInteractComponent::OnCapsuleComponentEndOverlap(
  UPrimitiveComponent *OverlappedComponent,
  AActor *OtherActor,
  UPrimitiveComponent *OtherComp,
  int32 OtherBodyIndex
) {
  if (bReportedInteractionAvailability && !CanInteract()) {
    bReportedInteractionAvailability = false;
    OnInteractionAvailability(false);
  }
}

TArray<ARedwoodInteractable *> URedwoodInteractComponent::GetInteractables() {
  ARedwoodCharacter *Character = Cast<ARedwoodCharacter>(GetOwner());
  if (!IsValid(Character)) {
    return TArray<ARedwoodInteractable *>();
  }

  UCapsuleComponent *CapsuleComponent = Character->GetCapsuleComponent();
  if (!IsValid(CapsuleComponent)) {
    return TArray<ARedwoodInteractable *>();
  }

  TArray<AActor *> OverlappingActors;
  CapsuleComponent->GetOverlappingActors(
    OverlappingActors, ARedwoodInteractable::StaticClass()
  );

  TArray<ARedwoodInteractable *> Interactables;
  for (AActor *Actor : OverlappingActors) {
    ARedwoodInteractable *Interactable = Cast<ARedwoodInteractable>(Actor);
    if (IsValid(Interactable)) {
      Interactables.Add(Interactable);
    }
  }

  return Interactables;
}

bool URedwoodInteractComponent::CanInteract() {
  return GetInteractables().Num() > 0;
}

ARedwoodInteractable *
URedwoodInteractComponent::PickInteractable_Implementation(
  const TArray<ARedwoodInteractable *> &Interactables
) {
  return Interactables.Num() > 0 ? Interactables[0] : nullptr;
}

void URedwoodInteractComponent::RPC_Interact_Implementation() {
  if (!CanInteract()) {
    UE_LOG(
      LogRedwood,
      Warning,
      TEXT(
        "Called RPC_Interact but we cannot interact (likely no available interactables for this character)"
      )
    );
    return;
  }

  ARedwoodInteractable *Interactable = PickInteractable(GetInteractables());

  if (IsValid(Interactable)) {
    ARedwoodCharacter *Character = Cast<ARedwoodCharacter>(GetOwner());
    if (IsValid(Character)) {
      Interactable->OnInteract(Character);
    }
  }
}

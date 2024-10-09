// Copyright Incanta Games. All Rights Reserved.

#include "Interactables/RedwoodInteractComponent.h"
#include "Interactables/RedwoodInteractable.h"
#include "RedwoodModule.h"

#include "Components/ShapeComponent.h"

URedwoodInteractComponent::URedwoodInteractComponent() {
  SetIsReplicatedByDefault(true);
}

void URedwoodInteractComponent::BeginPlay() {
  Super::BeginPlay();

  APawn *Pawn = Cast<APawn>(GetOwner());
  if (!IsValid(Pawn)) {
    return;
  }

  USceneComponent *RootComponent = Pawn->GetRootComponent();
  if (!IsValid(RootComponent)) {
    return;
  }

  UShapeComponent *ShapeComponent = Cast<UShapeComponent>(RootComponent);
  if (!IsValid(ShapeComponent)) {
    return;
  }

  if (!Pawn->HasAuthority()) {
    ShapeComponent->OnComponentBeginOverlap.AddDynamic(
      this, &URedwoodInteractComponent::OnComponentBeginOverlap
    );

    ShapeComponent->OnComponentEndOverlap.AddDynamic(
      this, &URedwoodInteractComponent::OnComponentEndOverlap
    );
  }
}

void URedwoodInteractComponent::OnInteractionAvailability_Implementation(
  bool bAvailable
) {}

void URedwoodInteractComponent::OnComponentBeginOverlap(
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

void URedwoodInteractComponent::OnComponentEndOverlap(
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
  APawn *Pawn = Cast<APawn>(GetOwner());
  if (!IsValid(Pawn)) {
    return TArray<ARedwoodInteractable *>();
  }

  USceneComponent *RootComponent = Pawn->GetRootComponent();
  if (!IsValid(RootComponent)) {
    return TArray<ARedwoodInteractable *>();
  }

  UShapeComponent *ShapeComponent = Cast<UShapeComponent>(RootComponent);
  if (!IsValid(ShapeComponent)) {
    return TArray<ARedwoodInteractable *>();
  }

  TArray<AActor *> OverlappingActors;
  ShapeComponent->GetOverlappingActors(
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
    APawn *Pawn = Cast<APawn>(GetOwner());
    if (IsValid(Pawn)) {
      URedwoodCharacterComponent *CharacterComponent =
        Pawn->GetComponentByClass<URedwoodCharacterComponent>();
      if (IsValid(CharacterComponent)) {
        Interactable->OnInteract(Pawn, CharacterComponent);
      } else {
        UE_LOG(
          LogRedwood,
          Error,
          TEXT("Could not find URedwoodCharacterComponent on Pawn %s"),
          *Pawn->GetName()
        );
      }
    }
  }
}

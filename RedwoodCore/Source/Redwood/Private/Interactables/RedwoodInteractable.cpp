// Copyright Incanta Games. All Rights Reserved.

#include "Interactables/RedwoodInteractable.h"
#include "GameFramework/Pawn.h"
#include "RedwoodCharacterComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

// add a sphere collision component to the interactable
ARedwoodInteractable::ARedwoodInteractable() {
  bReplicates = true;

  RootComponent =
    CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

  SphereComponent =
    CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
  SphereComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
  SphereComponent->SetGenerateOverlapEvents(true);
  SphereComponent->SetupAttachment(RootComponent);
}

void ARedwoodInteractable::GetLifetimeReplicatedProps(
  TArray<FLifetimeProperty> &OutLifetimeProps
) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(ARedwoodInteractable, bAutoInteract);
}

void ARedwoodInteractable::BeginPlay() {
  Super::BeginPlay();

  UGameplayMessageSubsystem &MessageSubsystem =
    UGameplayMessageSubsystem::Get(this);
  ListenerHandle = MessageSubsystem.RegisterListener(
    TAG_Redwood_Player_Interaction, this, &ARedwoodInteractable::OnInteraction
  );
}

void ARedwoodInteractable::OnInteraction(
  FGameplayTag InChannel, const FRedwoodPlayerInteraction &Message
) {
  if (IsValid(Message.Pawn) && SphereComponent->IsOverlappingActor(Message.Pawn) && IsValid(Message.CharacterComponent)) {
    OnInteract(Message.Pawn, Message.CharacterComponent);
  }
}

void ARedwoodInteractable::OnInteract_Implementation(
  APawn *Pawn, URedwoodCharacterComponent *CharacterComponent
) {}

// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodZoneSpawn.h"
#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"

ARedwoodZoneSpawn::ARedwoodZoneSpawn(const FObjectInitializer &ObjectInitializer
) :
  Super(ObjectInitializer) {
  RootComponent =
    CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
  SetRootComponent(RootComponent);

  // add a billboard component
  UBillboardComponent *BillboardComponent =
    CreateDefaultSubobject<UBillboardComponent>(TEXT("BillboardComponent"));
  BillboardComponent->SetupAttachment(RootComponent);

  // add a arrow component
  UArrowComponent *ArrowComponent =
    CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
  ArrowComponent->SetupAttachment(RootComponent);
  ArrowComponent->ArrowColor = FColor::Red;
  ArrowComponent->ArrowSize = 1.0f;
  ArrowComponent->ArrowLength = 100.0f;
  ArrowComponent->bTreatAsASprite = true;
  ArrowComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
}

FTransform ARedwoodZoneSpawn::GetSpawnTransform() {
  FTransform Transform = GetActorTransform();
  FVector Location = Transform.GetLocation();

  if (SpawnRadius > 0.0f) {
    float RandomAngle = FMath::FRand() * 360.0f;
    float RandomRadius = FMath::FRand() * SpawnRadius;

    Location += FVector(
      FMath::Cos(RandomAngle) * RandomRadius,
      FMath::Sin(RandomAngle) * RandomRadius,
      0.0f
    );
  }

  // Make sure the location is at ground level
  FHitResult HitResult;
  if (GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Location,
        Location - FVector(0.0f, 0.0f, 1000.0f),
        ECC_WorldStatic
      )) {
    Location = HitResult.Location;
  } else if (GetWorld()->LineTraceSingleByChannel(
               HitResult,
               Location + FVector(0.0f, 0.0f, 1000.0f),
               Location,
               ECC_WorldStatic
             )) {
    Location = HitResult.Location;
  }

  Location += FVector(
    0.0f, 0.0f, 100.0f
  ); // TODO: make sure they're above the ground for now

  Transform.SetLocation(Location);

  if (bRandomizeRotation) {
    Transform.SetRotation(FQuat(FRotator(0.0f, FMath::FRand() * 360.0f, 0.0f)));
  }

  return Transform;
}
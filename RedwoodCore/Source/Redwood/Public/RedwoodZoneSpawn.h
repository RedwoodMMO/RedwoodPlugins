// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RedwoodZoneSpawn.generated.h"

UCLASS(BlueprintType, Blueprintable)
class REDWOOD_API ARedwoodZoneSpawn : public AActor {
  GENERATED_BODY()

public:
  ARedwoodZoneSpawn(
    const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get()
  );

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  FTransform GetSpawnTransform();

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  FString ZoneName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  FString SpawnName = TEXT("default");

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  float SpawnRadius = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  bool bRandomizeRotation = false;
};

// Copyright Incanta Games. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "RedwoodClientExecCommand.generated.h"

UCLASS(BlueprintType)
class REDWOOD_API ARedwoodClientExecCommand : public AActor {
  GENERATED_BODY()

public:
  ARedwoodClientExecCommand() {
    bReplicates = true;
  }

  //~ Begin AActor Interface
  virtual void BeginPlay() override;
  //~ End AActor Interface

  // Override IsNetRelevantFor to make this actor always relevant
  virtual bool IsNetRelevantFor(
    const AActor *RealViewer,
    const AActor *ViewTarget,
    const FVector &SrcLocation
  ) const override {
    return true;
  }

  UPROPERTY(
    EditAnywhere,
    BlueprintReadWrite,
    Category = "Redwood",
    Meta = (ExposeOnSpawn = true),
    Replicated
  )
  FString Command;
};

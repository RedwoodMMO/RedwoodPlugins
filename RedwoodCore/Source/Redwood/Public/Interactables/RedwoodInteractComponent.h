// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "RedwoodInteractComponent.generated.h"

class ARedwoodInteractable;

UCLASS(BlueprintType, Blueprintable)
class REDWOOD_API URedwoodInteractComponent : public UActorComponent {
  GENERATED_BODY()

public:
  URedwoodInteractComponent();

  //~ Begin UActorComponent Interface
  virtual void BeginPlay() override;
  //~ End UActorComponent Interface

  UFUNCTION(BlueprintNativeEvent, Category = "Redwood")
  void OnInteractionAvailability(bool bAvailable);

  UFUNCTION(BlueprintPure, Category = "Redwood")
  TArray<ARedwoodInteractable *> GetInteractables();

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool CanInteract();

  UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Redwood")
  void RPC_Interact();

  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Redwood")
  ARedwoodInteractable *PickInteractable(
    const TArray<ARedwoodInteractable *> &Interactables
  );

private:
  bool bReportedInteractionAvailability = false;

  UFUNCTION()
  void OnComponentBeginOverlap(
    UPrimitiveComponent *OverlappedComponent,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult &SweepResult
  );

  UFUNCTION()
  void OnComponentEndOverlap(
    UPrimitiveComponent *OverlappedComponent,
    AActor *OtherActor,
    UPrimitiveComponent *OtherComp,
    int32 OtherBodyIndex
  );
};

// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "RedwoodPersistenceComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Redwood))
class REDWOOD_API URedwoodPersistenceComponent : public UActorComponent {
  GENERATED_BODY()

public:
  //~ Begin UActorComponent Interface
  virtual void BeginPlay() override;
  //~ End UActorComponent Interface

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  FString RedwoodId;

  UPROPERTY(
    BlueprintReadWrite,
    EditAnywhere,
    Category = "Redwood",
    meta = (AllowedTypes = "RedwoodPersistentItemAsset")
  )
  FPrimaryAssetId PersistentItem;

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void MarkTransformDirty() {
    bTransformDirty = true;
  }

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsTransformDirty() const {
    return bTransformDirty;
  }

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void MarkDataDirty() {
    bDataDirty = true;
  }

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsDataDirty() const {
    return bDataDirty;
  }

private:
  bool bTransformDirty = false;
  bool bDataDirty = false;
};

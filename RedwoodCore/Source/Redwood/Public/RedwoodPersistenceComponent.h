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

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void MarkAllDirty() {
    bTransformDirty = true;
    bDataDirty = true;
  }

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void ClearDirtyFlags() {
    bTransformDirty = false;
    bDataDirty = false;
  }

  // This is called by the URedwoodServerGameSubsystem
  // if this actor/component is spawned due to existing
  // persistence. If this isn't called manually then when
  // the component is spawned otherwise (i.e. through normal gameplay
  // or initially in the editor) it will be persisted regardless
  // of dirty flags.
  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void SkipInitialSave() {
    bDoInitialSave = false;
  }

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool ShouldDoInitialSave() const {
    return bDoInitialSave;
  }

private:
  bool bTransformDirty = false;
  bool bDataDirty = false;

  // this fla
  bool bDoInitialSave = true;
};

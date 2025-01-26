// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "RedwoodSyncComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE(
  FOnRedwoodSyncComponentInitiallySpawned,
  URedwoodSyncComponent,
  InitiallySpawned
);

UCLASS(
  Blueprintable,
  BlueprintType,
  ClassGroup = (Redwood),
  meta = (BlueprintSpawnableComponent)
)
class REDWOOD_API URedwoodSyncComponent : public UActorComponent {
  GENERATED_BODY()

public:
  //~ Begin UActorComponent Interface
  virtual void BeginPlay() override;
  //~ End UActorComponent Interface

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redwood")
  bool bStoreDataInActor = true;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redwood")
  bool bPersistChanges = true;

  // You can specify a unique identifier if you'd like, but
  // Redwood will generate a random one for you during BeginPlay
  // if you leave this empty
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redwood")
  FString RedwoodId;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redwood")
  bool bUseData = true;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redwood")
  FString DataVariableName = TEXT("Data");

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redwood")
  int32 LatestDataSchemaVersion;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redwood")
  bool bUseAnimationState = false;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redwood")
  FString AnimationStateVariableName = TEXT("AnimationState");

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redwood")
  int32 LatestAnimationStateSchemaVersion;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString RedwoodTypeId;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString ZoneName;

  // How often you want to automatically sync movement data
  // (transform only currently) in seconds. If this is < 0
  // then it will only sync when the movement is marked dirty.
  // If this is set to 0 then it will sync every frame.
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redwood")
  float MovementSyncIntervalSeconds = -1;

  // This is fired shortly after Begin Play on the server that initially
  // spawned this sync component via spawning the owning actor.
  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FOnRedwoodSyncComponentInitiallySpawned InitiallySpawned;

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void MarkMovementDirty() {
    bMovementDirty = true;
    if (bPersistChanges) {
      bMovementDirtyPersistence = true;
    }
  }

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsMovementDirty(bool bForPersistence) const {
    return bForPersistence ? bMovementDirtyPersistence : bMovementDirty;
  }

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void MarkDataDirty() {
    bDataDirty = true;
    if (bPersistChanges) {
      bDataDirtyPersistence = true;
    }
  }

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsDataDirty(bool bForPersistence) const {
    return bForPersistence ? bDataDirtyPersistence : bDataDirty;
  }

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void MarkAllDirty() {
    bMovementDirty = true;
    bDataDirty = true;

    if (bPersistChanges) {
      bMovementDirtyPersistence = true;
      bDataDirtyPersistence = true;
    }
  }

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void ClearDirtyFlags(bool bForPersistence) {
    if (bForPersistence) {
      bMovementDirtyPersistence = false;
      bDataDirtyPersistence = false;
    } else {
      bMovementDirty = false;
      bDataDirty = false;
    }
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

  double GetLastMovementSyncTime() const {
    return LastMovementSyncTime;
  }

  void SetLastMovementSyncTime(double NewTime) {
    LastMovementSyncTime = NewTime;
  }

private:
  void InitSyncComponent();

  bool bMovementDirty = false;
  bool bMovementDirtyPersistence = false;
  bool bDataDirty = false;
  bool bDataDirtyPersistence = false;

  bool bDoInitialSave = true;

  double LastMovementSyncTime = 0;
};

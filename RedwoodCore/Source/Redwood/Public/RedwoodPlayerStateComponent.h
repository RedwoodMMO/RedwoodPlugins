// Copyright Incanta LLC. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "RedwoodPlayerStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRedwoodPlayerStateUpdated);

UCLASS(BlueprintType, Blueprintable)
class REDWOOD_API URedwoodPlayerStateComponent : public UActorComponent {
  GENERATED_BODY()

public:
  URedwoodPlayerStateComponent(const FObjectInitializer &ObjectInitializer);

  // NOT AVAILABLE ON CLIENTS
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FRedwoodPlayerData RedwoodPlayer;
  // NOT AVAILABLE ON CLIENTS
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FRedwoodCharacterBackend RedwoodCharacter;

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkCharacterDataDirty() {
    bCharacterDataDirty = true;
  }

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  bool IsCharacterDataDirty() const {
    return bCharacterDataDirty;
  }

  //~ Begin UActorComponent Interface
  virtual void TickComponent(
    float DeltaTime,
    enum ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction
  ) override;
  //~ End UActorComponent Interface

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redwood")
  bool bFollowPawn = false;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  bool bClientReady = false;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  bool bServerReady = false;

  UFUNCTION(
    BlueprintCallable,
    Server,
    Reliable,
    WithValidation,
    Category = "Redwood|PlayerState"
  )
  void SetClientReady();

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void SetServerReady();

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void SetRedwoodPlayer(FRedwoodPlayerData InRedwoodPlayer);

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void SetRedwoodCharacter(FRedwoodCharacterBackend InRedwoodCharacter);

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  bool GetSpawnData(FTransform &Transform, FRotator &ControlRotation);

  UPROPERTY(BlueprintAssignable, Category = "Events")
  FOnRedwoodPlayerStateUpdated OnRedwoodCharacterUpdated;

  UPROPERTY(BlueprintAssignable, Category = "Events")
  FOnRedwoodPlayerStateUpdated OnRedwoodPlayerUpdated;

  bool bTransferring = false;

  void ClearDirtyFlags() {
    bCharacterDataDirty = false;
  }

private:
  TWeakObjectPtr<APlayerState> OwnerPlayerState = nullptr;

  bool bCharacterDataDirty = false;
};

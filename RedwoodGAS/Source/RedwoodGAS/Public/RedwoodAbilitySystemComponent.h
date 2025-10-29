// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "RedwoodPersistenceComponentInterface.h"

#include "RedwoodAbilitySystemComponent.generated.h"

UENUM(BlueprintType)
enum class ERedwoodASCInclusionMode : uint8 { Blacklist, Whitelist };

UCLASS(
  Blueprintable,
  BlueprintType,
  ClassGroup = (Redwood),
  meta = (BlueprintSpawnableComponent)
)
class REDWOODGAS_API URedwoodAbilitySystemComponent
  : public UAbilitySystemComponent,
    public IRedwoodPersistenceComponentInterface {
  GENERATED_BODY()

public:
  URedwoodAbilitySystemComponent(const FObjectInitializer &ObjectInitializer);

  //~ Begin UActorComponent Interface
  virtual void BeginPlay() override;
  //~ End UActorComponent Interface

  //~ Begin IRedwoodPersistenceComponentInterface Interface
  void AddPersistedData(TSharedPtr<FJsonObject> JsonObject, bool bForce)
    override;
  //~ End IRedwoodPersistenceComponentInterface Interface

protected:
  UFUNCTION()
  void HandleGameplayEffectApplied(
    UAbilitySystemComponent *ASC,
    const FGameplayEffectSpec &Spec,
    FActiveGameplayEffectHandle ActiveHandle
  );
  UFUNCTION()
  void HandleGameplayEffectRemoved(const FActiveGameplayEffect &ActiveEffect);

  //~ Begin UAbilitySystemComponent Interface
  virtual void OnGiveAbility(FGameplayAbilitySpec &AbilitySpec) override;
  virtual void OnRemoveAbility(FGameplayAbilitySpec &AbilitySpec) override;
  //~ End UAbilitySystemComponent Interface

public:
  /**
   * How many intervals of URedwoodGameMode(Base)::DatabasePersistenceInterval
   * should this ASC persist? This allows to you persist ASC data less frequently
   * than other data.
   *
   * For example, if you have a URedwoodGameMode(Base)::DatabasePersistenceInterval
   * of 0.5 seconds and you set this variable to 3, this ASC will persist every
   * 1.5 seconds (if there are changes). Keeping this set to 0 will not add any
   * extra delays.
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  int32 UpdateIntervals = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  ERedwoodASCInclusionMode AbilityInclusionMode =
    ERedwoodASCInclusionMode::Blacklist;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  TArray<TSubclassOf<UGameplayAbility>> AbilityInclusionArray;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  ERedwoodASCInclusionMode EffectInclusionMode =
    ERedwoodASCInclusionMode::Blacklist;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  TArray<TSubclassOf<UGameplayEffect>> EffectInclusionArray;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  ERedwoodASCInclusionMode AttributeInclusionMode =
    ERedwoodASCInclusionMode::Blacklist;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  TArray<FGameplayAttribute> AttributeInclusionArray;

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkDirty() {
    bDirty = true;
  }

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  bool IsDirty() const {
    return bDirty;
  }

protected:
  TSharedPtr<FJsonObject> SerializeASC();
  void DeserializeASC(TSharedPtr<FJsonObject> Data);

private:
  UFUNCTION()
  void OnControllerChanged(
    APawn *Pawn, AController *OldController, AController *NewController
  );

  UFUNCTION()
  void RedwoodPlayerStateCharacterUpdated();

  bool bDirty = false;
  uint32 UpdateIntervalCounter = 0;
};

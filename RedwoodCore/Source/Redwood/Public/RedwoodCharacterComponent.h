// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "Types/RedwoodTypes.h"

#include "RedwoodCharacterComponent.generated.h"

class USIOJsonObject;

UCLASS(
  Blueprintable,
  BlueprintType,
  ClassGroup = (Redwood),
  meta = (BlueprintSpawnableComponent)
)
class REDWOOD_API URedwoodCharacterComponent : public UActorComponent {
  GENERATED_BODY()

public:
  URedwoodCharacterComponent(const FObjectInitializer &ObjectInitializer);

  //~ Begin UActorComponent Interface
  virtual void BeginPlay() override;
  //~ End UActorComponent Interface

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodDynamicDelegate OnRedwoodCharacterUpdated;

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodDynamicDelegate OnRedwoodPlayerUpdated;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redwood")
  bool bStoreDataInActor = true;

  UPROPERTY(Replicated, BlueprintReadOnly, Category = "Redwood")
  FString RedwoodPlayerId;

  UPROPERTY(Replicated, BlueprintReadOnly, Category = "Redwood")
  FString RedwoodPlayerNickname;

  UPROPERTY(Replicated, BlueprintReadOnly, Category = "Redwood")
  FString RedwoodNameTag;

  UPROPERTY(Replicated, BlueprintReadOnly, Category = "Redwood")
  bool bSelectedGuildValid = false;

  UPROPERTY(Replicated, BlueprintReadOnly, Category = "Redwood")
  FRedwoodGuildInfo SelectedGuild;

  UPROPERTY(Replicated, BlueprintReadOnly, Category = "Redwood")
  FString RedwoodCharacterId;

  UPROPERTY(Replicated, BlueprintReadOnly, Category = "Redwood")
  FString RedwoodCharacterName;

  // PlayerData is not the same as the other character data in
  // below variables. This PlayerData is associated with the
  // PlayerIdentity, not the PlayerCharacter. This means that
  // it is the same for all characters of a player across all
  // realms. It's disabled by default as most developers likely
  // won't use it.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  bool bUsePlayerData = false;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  FString PlayerDataVariableName = TEXT("PlayerData");

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  bool bUseCharacterCreatorData = true;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  FString CharacterCreatorDataVariableName = TEXT("CharacterCreatorData");

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  int32 LatestCharacterCreatorDataSchemaVersion = 0;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  bool bUseMetadata = true;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  FString MetadataVariableName = TEXT("Metadata");

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  int32 LatestMetadataSchemaVersion = 0;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  bool bUseEquippedInventory = true;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  FString EquippedInventoryVariableName = TEXT("EquippedInventory");

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  int32 LatestEquippedInventorySchemaVersion = 0;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  bool bUseNonequippedInventory = true;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  FString NonequippedInventoryVariableName = TEXT("NonequippedInventory");

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  int32 LatestNonequippedInventorySchemaVersion = 0;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  bool bUseProgress = false;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  FString ProgressVariableName = TEXT("Progress");

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  int32 LatestProgressSchemaVersion = 0;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  bool bUseData = true;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  FString DataVariableName = TEXT("Data");

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  int32 LatestDataSchemaVersion = 0;

  // Only enable this if you're using a custom Ability System;
  // if you're using GAS, keep this false and use the URedwoodGASComponent
  // instead.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  bool bUseAbilitySystem = false;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  FString AbilitySystemVariableName = TEXT("AbilitySystem");

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  int32 LatestAbilitySystemSchemaVersion = 0;

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkPlayerDataDirty() {
    if (bUsePlayerData) {
      bPlayerDataDirty = true;
    }
  }

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkCharacterCreatorDataDirty() {
    if (bUseCharacterCreatorData) {
      bCharacterCreatorDataDirty = true;
    }
  }

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkMetadataDirty() {
    if (bUseMetadata) {
      bMetadataDirty = true;
    }
  }

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkEquippedInventoryDirty() {
    if (bUseEquippedInventory) {
      bEquippedInventoryDirty = true;
    }
  }

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkNonequippedInventoryDirty() {
    if (bUseNonequippedInventory) {
      bNonequippedInventoryDirty = true;
    }
  }

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkProgressDirty() {
    if (bUseProgress) {
      bProgressDirty = true;
    }
  }

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkDataDirty() {
    if (bUseData) {
      bDataDirty = true;
    }
  }

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkAbilitySystemDirty() {
    if (bUseAbilitySystem) {
      bAbilitySystemDirty = true;
    }
  }

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  bool IsPlayerDataDirty() const {
    return bPlayerDataDirty;
  }

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsCharacterCreatorDataDirty() const {
    return bCharacterCreatorDataDirty;
  }

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsMetadataDirty() const {
    return bMetadataDirty;
  }

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsEquippedInventoryDirty() const {
    return bEquippedInventoryDirty;
  }

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsNonequippedInventoryDirty() const {
    return bNonequippedInventoryDirty;
  }

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsProgressDirty() const {
    return bProgressDirty;
  }

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsDataDirty() const {
    return bDataDirty;
  }

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsAbilitySystemDirty() const {
    return bAbilitySystemDirty;
  }

  void ClearDirtyFlags() {
    bPlayerDataDirty = false;
    bCharacterCreatorDataDirty = false;
    bMetadataDirty = false;
    bEquippedInventoryDirty = false;
    bNonequippedInventoryDirty = false;
    bProgressDirty = false;
    bDataDirty = false;
    bAbilitySystemDirty = false;
  }

private:
  UFUNCTION()
  void OnControllerChanged(
    APawn *Pawn, AController *OldController, AController *NewController
  );

  UFUNCTION()
  void RedwoodPlayerStatePlayerUpdated();

  UFUNCTION(NetMulticast, Reliable)
  void MC_RedwoodPlayerUpdated();

  UFUNCTION()
  void RedwoodPlayerStateCharacterUpdated();

  UFUNCTION(NetMulticast, Reliable)
  void MC_RedwoodCharacterUpdated();

  bool bPlayerDataDirty = false;
  bool bCharacterCreatorDataDirty = false;
  bool bMetadataDirty = false;
  bool bEquippedInventoryDirty = false;
  bool bNonequippedInventoryDirty = false;
  bool bProgressDirty = false;
  bool bDataDirty = false;
  bool bAbilitySystemDirty = false;
};

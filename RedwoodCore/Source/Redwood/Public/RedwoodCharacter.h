// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "RedwoodCharacter.generated.h"

class USIOJsonObject;

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Redwood))
class REDWOOD_API ARedwoodCharacter : public ACharacter {
  GENERATED_BODY()

public:
  ARedwoodCharacter(const FObjectInitializer &ObjectInitializer);

  //~AActor interface
  virtual void BeginPlay() override;
  //~End of AActor interface

  //~APawn interface
  virtual void PossessedBy(AController *NewController) override;
  //~End of APawn interface

  UFUNCTION(BlueprintNativeEvent, Category = "Redwood")
  void OnRedwoodCharacterUpdated();

  UPROPERTY(Replicated, BlueprintReadOnly, Category = "Redwood")
  FString RedwoodPlayerId;

  UPROPERTY(Replicated, BlueprintReadOnly, Category = "Redwood")
  FString RedwoodCharacterId;

  // Also available with PlayerState.GetPlayerName()
  UPROPERTY(Replicated, BlueprintReadOnly, Category = "Redwood")
  FString RedwoodCharacterName;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  int32 LatestCharacterCreatorDataSchemaVersion = 0;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  int32 LatestMetadataSchemaVersion = 0;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  int32 LatestEquippedSchemaVersion = 0;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  int32 LatestNonequippedSchemaVersion = 0;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Redwood")
  int32 LatestDataSchemaVersion = 0;

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkCharacterCreatorDataDirty();

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkMetadataDirty();

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkEquippedInventoryDirty();

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkNonequippedInventoryDirty();

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkDataDirty();

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
  bool IsDataDirty() const {
    return bDataDirty;
  }

  void ClearDirtyFlags() {
    bCharacterCreatorDataDirty = false;
    bMetadataDirty = false;
    bEquippedInventoryDirty = false;
    bNonequippedInventoryDirty = false;
    bDataDirty = false;
  }

  USIOJsonObject *SerializeBackendData(FString VariableName);

private:
  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void RedwoodPlayerStateCharacterUpdated();

  void DeserializeBackendData(
    USIOJsonObject *SIOJsonObject,
    FString VariableName,
    int32 LatestSchemaVersion
  );

  bool bCharacterCreatorDataDirty = false;
  bool bMetadataDirty = false;
  bool bEquippedInventoryDirty = false;
  bool bNonequippedInventoryDirty = false;
  bool bDataDirty = false;
};

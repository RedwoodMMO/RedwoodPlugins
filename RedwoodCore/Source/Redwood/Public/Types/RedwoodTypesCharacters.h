// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodTypesCommon.h"

#include "RedwoodTypesCharacters.generated.h"

USTRUCT(BlueprintType)
struct FRedwoodCharacterBackend {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FDateTime CreatedAt;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FDateTime UpdatedAt;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bArchived = false;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FDateTime ArchivedAt;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString PlayerId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Name;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *CharacterCreatorData = nullptr;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Metadata = nullptr;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *EquippedInventory = nullptr;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *NonequippedInventory = nullptr;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data = nullptr;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *RedwoodData = nullptr;
};

USTRUCT(BlueprintType)
struct FRedwoodListCharactersOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRedwoodCharacterBackend> Characters;
};

typedef TDelegate<void(const FRedwoodListCharactersOutput &)>
  FRedwoodListCharactersOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodListCharactersOutputDynamicDelegate,
  FRedwoodListCharactersOutput,
  Data
);

USTRUCT(BlueprintType)
struct FRedwoodGetCharacterOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FRedwoodCharacterBackend Character;
};

typedef TDelegate<void(const FRedwoodGetCharacterOutput &)>
  FRedwoodGetCharacterOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodGetCharacterOutputDynamicDelegate, FRedwoodGetCharacterOutput, Data
);

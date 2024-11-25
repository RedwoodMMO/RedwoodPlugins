// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodTypesCommon.h"

#include "GameFramework/SaveGame.h"

#include "RedwoodTypesBlobs.generated.h"

USTRUCT(BlueprintType)
struct FRedwoodGetBlobOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<uint8> Blob;
};

typedef TDelegate<void(const FRedwoodGetBlobOutput &)>
  FRedwoodGetBlobOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodGetBlobOutputDynamicDelegate, FRedwoodGetBlobOutput, Data
);

USTRUCT(BlueprintType)
struct FRedwoodGetSaveGameOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USaveGame *SaveGame = nullptr;
};

typedef TDelegate<void(const FRedwoodGetSaveGameOutput &)>
  FRedwoodGetSaveGameOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodGetSaveGameOutputDynamicDelegate, FRedwoodGetSaveGameOutput, Data
);

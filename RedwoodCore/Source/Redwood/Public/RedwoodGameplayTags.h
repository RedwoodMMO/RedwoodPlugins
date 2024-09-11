// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "NativeGameplayTags.h"
#include "SIOJsonObject.h"

#include "RedwoodGameplayTags.generated.h"

class ARedwoodPlayerState;
class ARedwoodCharacter;

REDWOOD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Redwood_Shutdown_Instance);
USTRUCT(BlueprintType)
struct REDWOOD_API FRedwoodReason {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Reason;
};

REDWOOD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Redwood_Player_Left);
USTRUCT(BlueprintType)
struct REDWOOD_API FRedwoodPlayerLeft {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  APlayerController *PlayerController;
};

REDWOOD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Redwood_Player_InventoryChanged);
USTRUCT(BlueprintType)
struct REDWOOD_API FRedwoodPlayerInventoryChanged {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  ARedwoodPlayerState *PlayerState;
};

REDWOOD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Redwood_Player_Interaction);
USTRUCT(BlueprintType)
struct REDWOOD_API FRedwoodPlayerInteraction {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  ARedwoodCharacter *Character;
};

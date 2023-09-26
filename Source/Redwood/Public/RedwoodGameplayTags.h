// Copyright Incanta Games 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "GameFramework/PlayerState.h"
#include "SIOJsonObject.h"

#include "RedwoodGameplayTags.generated.h"

REDWOOD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Redwood_Shutdown_Instance);
USTRUCT(BlueprintType)
struct REDWOOD_API FRedwoodReason {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Reason;
};

REDWOOD_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Redwood_Player_Joined);
USTRUCT(BlueprintType)
struct REDWOOD_API FRedwoodPlayerJoined {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  APlayerState* PlayerState;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject* CharacterData;
};

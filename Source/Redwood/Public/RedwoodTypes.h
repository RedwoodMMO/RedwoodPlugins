// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SIOJsonObject.h"

#include "RedwoodTypes.generated.h"

USTRUCT(BlueprintType)
struct FRegion {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Name;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Ping;
};

USTRUCT(BlueprintType)
struct FRegionsChanged {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRegion> Regions;
};

USTRUCT(BlueprintType)
struct FDataCenterLatency {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Url;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<float> RTTs;
};

USTRUCT(BlueprintType)
struct FDataCenterLatencySort {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  float RTT;
};

UENUM(BlueprintType)
enum class ERedwoodAuthUpdateType : uint8 { Success, MustVerifyAccount, Error };

UENUM(BlueprintType)
enum class ERedwoodLobbyUpdateType : uint8 {
  JoinResponse,
  Update,
  Ready,
  TicketStale
};

USTRUCT(BlueprintType)
struct FRedwoodPlayerCharacter {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data;
};

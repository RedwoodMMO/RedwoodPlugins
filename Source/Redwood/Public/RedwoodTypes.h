// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RedwoodTypes.generated.h"

USTRUCT(BlueprintType)
struct FRegion {
  GENERATED_BODY()

  UPROPERTY()
  FString Name;

  UPROPERTY()
  FString Ping;
};

USTRUCT(BlueprintType)
struct FRegionsChanged {
  GENERATED_BODY()

  UPROPERTY()
  TArray<FRegion> Regions;
};

USTRUCT(BlueprintType)
struct FDataCenterLatency {
  GENERATED_BODY()

  FString Id;
  FString Url;
  TArray<float> RTTs;
};

USTRUCT(BlueprintType)
struct FDataCenterLatencySort {
  GENERATED_BODY()

  FString Id;
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

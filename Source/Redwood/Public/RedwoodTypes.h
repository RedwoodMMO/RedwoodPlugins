// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SIOJsonObject.h"

#include "RedwoodTypes.generated.h"

USTRUCT(BlueprintType)
struct FRedwoodRegion {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Name;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Ping;
};

USTRUCT(BlueprintType)
struct FRedwoodRegionsChanged {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRedwoodRegion> Regions;
};

USTRUCT(BlueprintType)
struct FRedwoodRegionLatency {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Url;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<float> RTTs;
};

USTRUCT(BlueprintType)
struct FRedwoodRegionLatencySort {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  float RTT = 0.0f;
};

UENUM(BlueprintType)
enum class ERedwoodAuthUpdateType : uint8 {
  Success,
  MustVerifyAccount,
  Error,
  Unknown
};

UENUM(BlueprintType)
enum class ERedwoodTicketingUpdateType : uint8 {
  JoinResponse,
  Update,
  Ready,
  TicketStale,
  Unknown
};

USTRUCT(BlueprintType)
struct FRedwoodPlayerCharacter {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data;
};

USTRUCT(BlueprintType)
struct FRedwoodAuthUpdate {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  ERedwoodAuthUpdateType Type = ERedwoodAuthUpdateType::Unknown;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Message;
};

USTRUCT(BlueprintType)
struct FRedwoodCharactersResult {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  TArray<FRedwoodPlayerCharacter> Characters;
};

USTRUCT(BlueprintType)
struct FRedwoodCharacterResult {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FRedwoodPlayerCharacter Character;
};

USTRUCT(BlueprintType)
struct FRedwoodTicketingUpdate {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  ERedwoodTicketingUpdateType Type = ERedwoodTicketingUpdateType::Unknown;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Message;
};

USTRUCT(BlueprintType)
struct FRedwoodRealm {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Name;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Uri;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString PingHost;
};

USTRUCT(BlueprintType)
struct FRedwoodRealmsResult {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  bool bSingleRealm = false;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  TArray<FRedwoodRealm> Realms;
};

USTRUCT(BlueprintType)
struct FRedwoodSocketConnected {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString SocketId;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString SessionId;
};

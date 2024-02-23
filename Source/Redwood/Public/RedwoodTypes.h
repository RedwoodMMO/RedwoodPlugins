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
  USIOJsonObject *Data = nullptr;
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

USTRUCT(BlueprintType)
struct FRedwoodGameServerProxy {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FDateTime CreatedAt;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FDateTime UpdatedAt;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FDateTime EndedAt;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Name;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Region;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Mode;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  bool bPublic = false;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  bool bContinuousPlay = false;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  bool bHasPassword = false;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Password;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString ShortCode;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  int32 CurrentPlayers = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  int32 MaxPlayers = 0;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data = nullptr;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString OwnerPlayerIdentityId;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString ActiveInstanceId;
};

USTRUCT(BlueprintType)
struct FRedwoodGameServerInstance {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FDateTime CreatedAt;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FDateTime UpdatedAt;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString ProviderId;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FDateTime StartedAt;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FDateTime EndedAt;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Connection;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString ContainerId;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString ProxyId;
};

USTRUCT(BlueprintType)
struct FRedwoodCreateServerParameters {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Name;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Region;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Mode;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  bool bPublic = false;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  bool bContinuousPlay = false;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Password;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString ShortCode;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  int32 MaxPlayers = 0;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data = nullptr;
};

USTRUCT(BlueprintType)
struct FRedwoodListServers {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  TArray<FRedwoodGameServerProxy> Servers;
};

USTRUCT(BlueprintType)
struct FRedwoodGetServer {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FRedwoodGameServerInstance ServerInstance;
};

USTRUCT(BlueprintType)
struct FRedwoodSimpleResult {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Error;
};

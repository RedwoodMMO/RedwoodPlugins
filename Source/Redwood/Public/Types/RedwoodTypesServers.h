// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodTypesCommon.h"

#include "RedwoodTypesServers.generated.h"

USTRUCT(BlueprintType)
struct FRedwoodGameServerProxy {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FDateTime CreatedAt;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FDateTime UpdatedAt;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FDateTime EndedAt;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Name;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Region;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString ModeId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString MapId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bPublic = false;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bContinuousPlay = false;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bHasPassword = false;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Password;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString ShortCode;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  int32 CurrentPlayers = 0;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  int32 MaxPlayers = 0;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data = nullptr;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString OwnerPlayerIdentityId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString ActiveInstanceId;
};

USTRUCT(BlueprintType)
struct FRedwoodGameServerInstance {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FDateTime CreatedAt;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FDateTime UpdatedAt;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString ProviderId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FDateTime StartedAt;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FDateTime EndedAt;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Connection;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString ContainerId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString ProxyId;
};

USTRUCT(BlueprintType)
struct FRedwoodCreateServerInput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Name;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Region;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString ModeId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString MapId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bPublic = false;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bContinuousPlay = false;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Password;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString ShortCode;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  int32 MaxPlayers = 0;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data = nullptr;
};

USTRUCT(BlueprintType)
struct FRedwoodListServersOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRedwoodGameServerProxy> Servers;
};

typedef TDelegate<void(const FRedwoodListServersOutput &)>
  FRedwoodListServersOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodListServersOutputDynamicDelegate, FRedwoodListServersOutput, Data
);

USTRUCT(BlueprintType)
struct FRedwoodCreateServerOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString ServerReference;
};

typedef TDelegate<void(const FRedwoodCreateServerOutput &)>
  FRedwoodCreateServerOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodCreateServerOutputDynamicDelegate, FRedwoodCreateServerOutput, Data
);

USTRUCT(BlueprintType)
struct FRedwoodGetServerOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FRedwoodGameServerInstance ServerInstance;
};

typedef TDelegate<void(const FRedwoodGetServerOutput &)>
  FRedwoodGetServerOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodGetServerOutputDynamicDelegate, FRedwoodGetServerOutput, Data
);

// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodTypesCommon.h"
#include "RedwoodTypesServers.h"

#include "RedwoodTypesFriends.generated.h"

UENUM(BlueprintType)
enum class ERedwoodFriendListType : uint8 {
  All,
  Active,
  PendingAll,
  PendingReceived,
  PendingSent,
  Blocked,
  Inactive,
  Unknown
};

USTRUCT(BlueprintType)
struct FRedwoodPlayerOnlineStateRealm : public FRedwoodServerDetails {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString CharacterId;
};

USTRUCT(BlueprintType)
struct FRedwoodFriend {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString PlayerId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Nickname;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  ERedwoodFriendListType State;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bOnline = false;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bPlaying = false;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FRedwoodPlayerOnlineStateRealm OnlineStateRealm;
};

USTRUCT(BlueprintType)
struct FRedwoodListFriendsOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRedwoodFriend> Players;
};

typedef TDelegate<void(const FRedwoodListFriendsOutput &)>
  FRedwoodListFriendsOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodListFriendsOutputDynamicDelegate, FRedwoodListFriendsOutput, Data
);

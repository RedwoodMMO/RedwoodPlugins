// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RedwoodTypesChat.generated.h"

UENUM(BlueprintType)
enum class ERedwoodChatRoomType : uint8 {
  Guild,
  Party,
  Proxy,
  Shard,
  Team,
  Nearby,
  Direct,
  Unknown
};

USTRUCT(BlueprintType)
struct FRedwoodChatIdentity {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FString PlayerId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FString Nickname;
};

USTRUCT(BlueprintType)
struct FRedwoodChatRoomIdentity {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FString CompleteRoomId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  ERedwoodChatRoomType Type = ERedwoodChatRoomType::Unknown;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FString RedwoodId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FString Name;
};

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodChatJoinPrivateRoomDynamicDelegate,
  const FRedwoodChatRoomIdentity &,
  RoomIdentity
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
  FRedwoodChatPrivateChatReceivedDynamicDelegate,
  const FRedwoodChatIdentity &,
  Sender,
  const FDateTime &,
  Timestamp,
  const FString &,
  Message
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
  FRedwoodChatRoomChatReceivedDynamicDelegate,
  const FRedwoodChatRoomIdentity &,
  RoomIdentity,
  const FRedwoodChatIdentity &,
  Sender,
  const FDateTime &,
  Timestamp,
  const FString &,
  Message,
  const FVector &,
  Location
);

// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RedwoodTypesChat.generated.h"

UENUM(BlueprintType)
enum class ERedwoodChatRoomType : uint8 {
  Guild,
  Party,
  Realm,
  Proxy,
  Shard,
  Team,
  Nearby,
  Custom,
  Direct,
  Unknown
};

/**
 * How a custom room lets people in who have not been invited. Invitations work
 * under all three.
 */
UENUM(BlueprintType)
enum class ERedwoodChatRoomJoinPolicy : uint8 {
  /** Nobody joins themselves; an invitation is the only way in. */
  Invite,
  /** Anyone with the room name and its join code. */
  Code,
  /** Anyone with the room name. */
  Open
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

/** One message, as retrieved from history rather than pushed live. */
USTRUCT(BlueprintType)
struct FRedwoodChatMessage {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FString MessageId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FRedwoodChatRoomIdentity Room;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FRedwoodChatIdentity Sender;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FDateTime Timestamp;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FString Message;
};

/** A channel with messages the player has not seen. */
USTRUCT(BlueprintType)
struct FRedwoodChatUnreadChannel {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FRedwoodChatRoomIdentity Room;

  /**
   * Who or what to label the channel with: the other party in a one-to-one
   * conversation, the guild or room name otherwise.
   */
  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FString DisplayName;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  int32 UnreadCount = 0;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FString OldestUnreadMessageId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FString OldestUnreadBody;
};

/** A custom room the player belongs to, or has been invited to. */
USTRUCT(BlueprintType)
struct FRedwoodChatRoom {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FString RoomId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FString Name;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  int32 MemberCount = 0;

  /** 0 is an ordinary member; anything higher administers the room. */
  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  int32 Role = 0;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  ERedwoodChatRoomJoinPolicy JoinPolicy = ERedwoodChatRoomJoinPolicy::Invite;

  /** Only populated for room admins, who are the ones who pass it on. */
  UPROPERTY(BlueprintReadWrite, Category = "Redwood Chat")
  FString JoinCode;
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

// Two flavours of each callback, following the convention in RedwoodTypes.h: a
// plain delegate for the C++ call, and a dynamic multicast for the Blueprint
// async node's assignable output pin.

typedef TDelegate<void(const FString &, const TArray<FRedwoodChatMessage> &)>
  FRedwoodChatHistoryOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
  FRedwoodChatHistoryDynamicDelegate,
  const FString &,
  Error,
  const TArray<FRedwoodChatMessage> &,
  Messages
);

typedef TDelegate<
  void(const FString &, const TArray<FRedwoodChatUnreadChannel> &)>
  FRedwoodChatUnreadOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
  FRedwoodChatUnreadDynamicDelegate,
  const FString &,
  Error,
  const TArray<FRedwoodChatUnreadChannel> &,
  Channels
);

typedef TDelegate<void(const FString &, const TArray<FRedwoodChatRoom> &)>
  FRedwoodChatRoomListOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
  FRedwoodChatRoomListDynamicDelegate,
  const FString &,
  Error,
  const TArray<FRedwoodChatRoom> &,
  Rooms
);

typedef TDelegate<void(const FString &, const FString &)>
  FRedwoodChatRoomCreatedOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
  FRedwoodChatRoomCreatedDynamicDelegate,
  const FString &,
  Error,
  const FString &,
  JoinCode
);

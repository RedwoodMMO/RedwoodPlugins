// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodChatModule.h"

#include "CoreMinimal.h"
#include "SocketIONative.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/RedwoodTypes.h"
#include "Types/RedwoodTypesChat.h"

#include "RedwoodChatClientSubsystem.generated.h"

/**
 * Player-facing chat.
 *
 * Chat rides the two Redwood connections the player already has rather than a
 * server of its own. Which connection carries a channel is what decides whether
 * it is account space or character space:
 *
 *   Director  accounts    direct-to-player, guild, account rooms
 *   Realm     characters  direct-to-character, realm, party, proxy, shard,
 *                         nearby, realm rooms
 *
 * Two behaviours worth knowing:
 *
 * - Ambient channels (realm, proxy, shard, nearby) only arrive while you have
 *   joined them. Joining is a subscription rather than a presence: nobody is
 *   told you are there, and nothing is waiting for you when you return.
 *   Joining a durable channel succeeds and does nothing, so a caller need not
 *   know which kind a channel is.
 *
 * - Nearby messages are NOT sent from here. They need a position the server
 *   trusts and an audience worked out from the world, so they originate on the
 *   game server; see URedwoodServerGameSubsystem::SendNearbyChatMessage. Your
 *   game supplies the one hop this subsystem cannot: a Server RPC from the
 *   player to their game server.
 */
UCLASS(BlueprintType)
class REDWOODCHAT_API URedwoodClientChatSubsystem
  : public UGameInstanceSubsystem {
  GENERATED_BODY()

public:
  // Begin USubsystem
  virtual void Initialize(FSubsystemCollectionBase &Collection) override;
  virtual void Deinitialize() override;
  // End USubsystem

  /** True once chat is listening on the Director connection at least. */
  UFUNCTION(BlueprintPure, Category = "Redwood Chat")
  bool IsConnected();

  /**
   * Start listening. The player must already be logged into the Director;
   * realm channels start working once they are connected to a realm and have
   * selected a character.
   */
  void InitializeChatConnection(FRedwoodErrorOutputDelegate OnOutput);

  /**
   * Ask to start receiving a channel. Required for realm, proxy, shard, and
   * nearby; harmless on the rest, which are always delivered.
   */
  UFUNCTION(BlueprintCallable, Category = "Redwood Chat")
  void JoinRoom(ERedwoodChatRoomType Type, FString Id);

  /**
   * Join a custom room by name. `Password` is the room's join code, empty if it
   * has none. `bJoinAsCharacter` chooses between a room local to your realm and
   * one that follows your account.
   */
  UFUNCTION(BlueprintCallable, Category = "Redwood Chat")
  void JoinCustomRoom(FString Id, FString Password, bool bJoinAsCharacter);

  /** Stop receiving a channel. On a custom room this leaves it for good. */
  UFUNCTION(BlueprintCallable, Category = "Redwood Chat")
  void LeaveRoom(ERedwoodChatRoomType Type, FString Id);

  UFUNCTION(BlueprintCallable, Category = "Redwood Chat")
  void SendMessageToRoom(
    ERedwoodChatRoomType Type, FString Id, const FString &Message
  );

  /** Message another account. They receive it wherever they are. */
  UFUNCTION(BlueprintCallable, Category = "Redwood Chat")
  void SendMessageToPlayer(
    const FString &TargetPlayerId, const FString &Message
  );

  /** Message another character in your realm, without naming their account. */
  UFUNCTION(BlueprintCallable, Category = "Redwood Chat")
  void SendMessageToCharacter(
    const FString &TargetCharacterId, const FString &Message
  );

  /**
   * Create a custom room. Supplying a password makes it joinable by anyone with
   * the name and that code; leaving it empty makes it joinable by name alone.
   * The code actually in force comes back through the delegate, since the
   * server generates one if you do not supply it.
   */
  void CreateCustomRoom(
    FString Id,
    FString Password,
    bool bCreateAsCharacter,
    FRedwoodChatRoomCreatedOutputDelegate OnOutput
  );

  /** Rooms you belong to. */
  void ListRooms(bool bAsCharacter, FRedwoodChatRoomListOutputDelegate OnOutput);

  /** Invitations awaiting an answer. */
  void ListRoomInvites(
    bool bAsCharacter, FRedwoodChatRoomListOutputDelegate OnOutput
  );

  UFUNCTION(BlueprintCallable, Category = "Redwood Chat")
  void RespondToRoomInvite(FString RoomId, bool bAsCharacter, bool bAccept);

  UFUNCTION(BlueprintCallable, Category = "Redwood Chat")
  void InviteToRoom(FString RoomId, bool bAsCharacter, FString MemberId);

  /**
   * Earlier messages in a channel, newest first. Pass the id of the oldest
   * message you already have as `Before` to page further back, or leave it
   * empty for the most recent page.
   */
  void GetHistory(
    ERedwoodChatRoomType Type,
    FString Id,
    FString Before,
    FRedwoodChatHistoryOutputDelegate OnOutput
  );

  /**
   * Channels with messages you have not seen, on both connections. Ambient
   * channels never appear: nothing there was addressed to you.
   */
  void GetUnreadSummary(FRedwoodChatUnreadOutputDelegate OnOutput);

  /** Mark a channel read up to and including a message you have seen. */
  UFUNCTION(BlueprintCallable, Category = "Redwood Chat")
  void MarkRead(ERedwoodChatRoomType Type, FString Id, FString UpToMessageId);

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodChatJoinPrivateRoomDynamicDelegate OnJoinPrivateRoom;

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodChatPrivateChatReceivedDynamicDelegate OnPlayerPrivateChatReceived;

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodChatPrivateChatReceivedDynamicDelegate OnCharacterPrivateChatReceived;

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodChatRoomChatReceivedDynamicDelegate OnRoomChatReceived;

  static FString SerializeRoomType(ERedwoodChatRoomType Type) {
    switch (Type) {
      case ERedwoodChatRoomType::Guild:
        return TEXT("guild");
      case ERedwoodChatRoomType::Realm:
        return TEXT("realm");
      case ERedwoodChatRoomType::Party:
        return TEXT("party");
      case ERedwoodChatRoomType::Proxy:
        return TEXT("proxy");
      case ERedwoodChatRoomType::Shard:
        return TEXT("shard");
      case ERedwoodChatRoomType::Team:
        return TEXT("team");
      case ERedwoodChatRoomType::Nearby:
        return TEXT("nearby");
      case ERedwoodChatRoomType::Custom:
        return TEXT("custom");
      case ERedwoodChatRoomType::Direct:
        return TEXT("direct");
      default:
        return TEXT("unknown");
    }
  }

  static ERedwoodChatRoomType ParseRoomType(const FString &RoomTypeString) {
    if (RoomTypeString == TEXT("guild")) {
      return ERedwoodChatRoomType::Guild;
    } else if (RoomTypeString == TEXT("party")) {
      return ERedwoodChatRoomType::Party;
    } else if (RoomTypeString == TEXT("realm")) {
      return ERedwoodChatRoomType::Realm;
    } else if (RoomTypeString == TEXT("proxy")) {
      return ERedwoodChatRoomType::Proxy;
    } else if (RoomTypeString == TEXT("shard")) {
      return ERedwoodChatRoomType::Shard;
    } else if (RoomTypeString == TEXT("team")) {
      return ERedwoodChatRoomType::Team;
    } else if (RoomTypeString == TEXT("nearby")) {
      return ERedwoodChatRoomType::Nearby;
    } else if (RoomTypeString == TEXT("custom")) {
      return ERedwoodChatRoomType::Custom;
    } else if (RoomTypeString == TEXT("direct")) {
      return ERedwoodChatRoomType::Direct;
    }
    return ERedwoodChatRoomType::Unknown;
  }

private:
  bool bInitialized = false;

  /**
   * Which connection owns a channel type. Custom rooms exist on both, so the
   * caller says which one it means; everything else is decided by the channel.
   */
  TSharedPtr<FSocketIONative> ConnectionFor(
    ERedwoodChatRoomType Type, bool bCharacterSpace
  ) const;

  /** True when this channel type lives on the realm connection. */
  static bool IsCharacterSpace(ERedwoodChatRoomType Type);

  void BindReceive(TSharedPtr<FSocketIONative> Connection, bool bCharacterSpace);

  void HandleReceived(
    const TSharedPtr<FJsonObject> &Message, bool bCharacterSpace
  );

  TSharedPtr<FJsonObject> MakeRequest(bool bCharacterSpace) const;

  TSharedPtr<FSocketIONative> Director;
  TSharedPtr<FSocketIONative> Realm;
  FString PlayerId;

  /**
   * Which space each custom room was joined in, so later sends and leaves go
   * back to the same place. A room name can exist on both.
   */
  TMap<FString, bool> CustomRoomUsesCharacter;
};

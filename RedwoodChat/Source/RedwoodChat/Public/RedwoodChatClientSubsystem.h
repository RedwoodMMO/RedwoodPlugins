// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodChatModule.h"

#include "CoreMinimal.h"
#include "SocketIONative.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/RedwoodTypes.h"
#include "Types/RedwoodTypesChat.h"
#include "XmppChat.h"
#include "XmppConnection.h"

#include "RedwoodChatClientSubsystem.generated.h"

UCLASS(BlueprintType)
class REDWOODCHAT_API URedwoodChatClientSubsystem
  : public UGameInstanceSubsystem {
  GENERATED_BODY()

public:
  // Begin USubsystem
  virtual void Initialize(FSubsystemCollectionBase &Collection) override;
  virtual void Deinitialize() override;
  // End USubsystem

  UFUNCTION(BlueprintPure, Category = "Redwood Chat")
  bool IsConnected();

  void InitializeChatConnection(FRedwoodErrorOutputDelegate OnOutput);

  UFUNCTION(BlueprintCallable, Category = "Redwood Chat")
  void JoinRoom(ERedwoodChatRoomType Type, FString Id);

  UFUNCTION(BlueprintCallable, Category = "Redwood Chat")
  void SendMessageToRoom(
    ERedwoodChatRoomType Type, FString Id, const FString &Message
  );

  UFUNCTION(BlueprintCallable, Category = "Redwood Chat")
  void SendMessageToPlayer(
    const FString &TargetPlayerId, const FString &Message
  );

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodChatJoinPrivateRoomDynamicDelegate OnJoinPrivateRoom;

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodChatPrivateChatReceivedDynamicDelegate OnPrivateChatReceived;

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodChatRoomChatReceivedDynamicDelegate OnRoomChatReceived;

  static FString SerializeRoomType(ERedwoodChatRoomType Type) {
    switch (Type) {
      case ERedwoodChatRoomType::Guild:
        return TEXT("guild");
      case ERedwoodChatRoomType::Party:
        return TEXT("party");
      case ERedwoodChatRoomType::Proxy:
        return TEXT("proxy");
      case ERedwoodChatRoomType::Shard:
        return TEXT("shard");
      case ERedwoodChatRoomType::Team:
        return TEXT("team");
      default:
        return TEXT("unknown");
    }
  }

  static ERedwoodChatRoomType ParseRoomType(const FString &RoomTypeString) {
    if (RoomTypeString == TEXT("guild")) {
      return ERedwoodChatRoomType::Guild;
    } else if (RoomTypeString == TEXT("party")) {
      return ERedwoodChatRoomType::Party;
    } else if (RoomTypeString == TEXT("proxy")) {
      return ERedwoodChatRoomType::Proxy;
    } else if (RoomTypeString == TEXT("shard")) {
      return ERedwoodChatRoomType::Shard;
    } else if (RoomTypeString == TEXT("team")) {
      return ERedwoodChatRoomType::Team;
    }
    return ERedwoodChatRoomType::Unknown;
  }

private:
  void InitHandlers();

  void HandlePrivateChatReceiveMessage(
    const TSharedRef<IXmppConnection> &Connection,
    const FXmppUserJid &InUserJid,
    const TSharedRef<FXmppChatMessage> &Message
  );
  void HandleJoinPrivateRoom(
    const TSharedRef<IXmppConnection> &Connection,
    bool bSuccess,
    const FString &RoomId,
    const FString &Error
  );
  void HandleRoomChatReceived(
    const TSharedRef<IXmppConnection> &Connection,
    const FString &RoomId,
    const FXmppUserJid &InUserJid,
    const TSharedRef<FXmppChatMessage> &ChatMsg
  );

  TSharedPtr<FSocketIONative> Director;
  FString PlayerId;
  FString Nickname;
  FString XmppPassword;

  TSharedPtr<class IXmppConnection> XmppConnection;
  FXmppUserJid UserJid;
};

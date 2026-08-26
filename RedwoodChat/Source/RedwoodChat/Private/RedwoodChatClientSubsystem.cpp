// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodChatClientSubsystem.h"
#include "RedwoodClientGameSubsystem.h"
#include "RedwoodClientInterface.h"

namespace {

/** Pull an ISO timestamp out of a payload, falling back to now. */
FDateTime ParseTimestamp(const TSharedPtr<FJsonObject> &Object) {
  FString Raw;
  FDateTime Parsed;
  if (Object->TryGetStringField(TEXT("createdAt"), Raw) &&
      FDateTime::ParseIso8601(*Raw, Parsed)) {
    return Parsed;
  }
  return FDateTime::UtcNow();
}

FString ErrorOf(const TSharedPtr<FJsonObject> &Object) {
  FString Error;
  Object->TryGetStringField(TEXT("error"), Error);
  return Error;
}

ERedwoodChatRoomJoinPolicy ParseJoinPolicy(const FString &Raw) {
  if (Raw == TEXT("code")) {
    return ERedwoodChatRoomJoinPolicy::Code;
  }
  if (Raw == TEXT("open")) {
    return ERedwoodChatRoomJoinPolicy::Open;
  }
  return ERedwoodChatRoomJoinPolicy::Invite;
}

/** Rooms come back the same shape from both the list and the invite list. */
TArray<FRedwoodChatRoom> ParseRooms(const TSharedPtr<FJsonObject> &Object) {
  TArray<FRedwoodChatRoom> Rooms;

  const TArray<TSharedPtr<FJsonValue>> *Values;
  if (!Object->TryGetArrayField(TEXT("rooms"), Values)) {
    return Rooms;
  }

  for (const TSharedPtr<FJsonValue> &Value : *Values) {
    const TSharedPtr<FJsonObject> Entry = Value->AsObject();
    if (!Entry.IsValid()) {
      continue;
    }

    FRedwoodChatRoom Room;
    Entry->TryGetStringField(TEXT("channelKey"), Room.RoomId);
    Entry->TryGetStringField(TEXT("name"), Room.Name);
    Entry->TryGetNumberField(TEXT("memberCount"), Room.MemberCount);
    Entry->TryGetNumberField(TEXT("role"), Room.Role);
    Entry->TryGetStringField(TEXT("joinCode"), Room.JoinCode);

    FString Policy;
    Entry->TryGetStringField(TEXT("joinPolicy"), Policy);
    Room.JoinPolicy = ParseJoinPolicy(Policy);

    Rooms.Add(Room);
  }

  return Rooms;
}

} // namespace

void URedwoodClientChatSubsystem::Initialize(
  FSubsystemCollectionBase &Collection
) {
  Super::Initialize(Collection);
}

void URedwoodClientChatSubsystem::Deinitialize() {
  Director.Reset();
  Realm.Reset();
  bInitialized = false;

  Super::Deinitialize();
}

void URedwoodClientChatSubsystem::InitializeChatConnection(
  FRedwoodErrorOutputDelegate OnOutput
) {
  if (bInitialized) {
    OnOutput.ExecuteIfBound(TEXT("Already initialized."));
    return;
  }

  URedwoodClientGameSubsystem *GameSubsystem =
    GetGameInstance()->GetSubsystem<URedwoodClientGameSubsystem>();
  if (!GameSubsystem) {
    OnOutput.ExecuteIfBound(TEXT("Redwood Client Game Subsystem not found."));
    return;
  }

  URedwoodClientInterface *ClientInterface =
    GameSubsystem->GetClientInterface();
  if (!ClientInterface) {
    OnOutput.ExecuteIfBound(TEXT("Redwood Client Interface not found."));
    return;
  }

  if (!ClientInterface->IsDirectorConnected()) {
    OnOutput.ExecuteIfBound(TEXT("Not connected to the Director."));
    return;
  }

  // No credentials to fetch and no second server to log into: chat listens on
  // the connections the player already has.
  PlayerId = ClientInterface->GetPlayerId();
  Director = ClientInterface->GetDirectorConnection();
  Realm = ClientInterface->GetRealmConnection();

  BindReceive(Director, false);

  // The realm connection is optional here. A player sitting in the main menu
  // has no realm and no character, and correctly belongs to no character-space
  // channel; realm chat starts working once they are in one.
  if (Realm.IsValid()) {
    BindReceive(Realm, true);
  }

  bInitialized = true;
  OnOutput.ExecuteIfBound(TEXT(""));
}

bool URedwoodClientChatSubsystem::IsConnected() {
  return bInitialized && Director.IsValid() && Director->bIsConnected;
}

bool URedwoodClientChatSubsystem::IsCharacterSpace(ERedwoodChatRoomType Type) {
  switch (Type) {
    case ERedwoodChatRoomType::Realm:
    case ERedwoodChatRoomType::Party:
    case ERedwoodChatRoomType::Proxy:
    case ERedwoodChatRoomType::Shard:
    case ERedwoodChatRoomType::Nearby:
    case ERedwoodChatRoomType::Team:
      return true;
    default:
      // Guild and account rooms are account space; Custom and Direct exist in
      // both and are decided by the caller.
      return false;
  }
}

TSharedPtr<FSocketIONative> URedwoodClientChatSubsystem::ConnectionFor(
  ERedwoodChatRoomType Type, bool bCharacterSpace
) const {
  const bool bUseRealm = IsCharacterSpace(Type) || bCharacterSpace;
  return bUseRealm ? Realm : Director;
}

TSharedPtr<FJsonObject> URedwoodClientChatSubsystem::MakeRequest(
  bool bCharacterSpace
) const {
  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  return Payload;
}

void URedwoodClientChatSubsystem::BindReceive(
  TSharedPtr<FSocketIONative> Connection, bool bCharacterSpace
) {
  if (!Connection.IsValid()) {
    return;
  }

  Connection->OnEvent(
    TEXT("chat:received"),
    [this, bCharacterSpace](
      const FString &Event, const TSharedPtr<FJsonValue> &Message
    ) {
      const TSharedPtr<FJsonObject> Object = Message->AsObject();
      if (Object.IsValid()) {
        HandleReceived(Object, bCharacterSpace);
      }
    }
  );
}

void URedwoodClientChatSubsystem::HandleReceived(
  const TSharedPtr<FJsonObject> &Message, bool bCharacterSpace
) {
  FString ChannelTypeString;
  Message->TryGetStringField(TEXT("channelType"), ChannelTypeString);
  const ERedwoodChatRoomType Type = ParseRoomType(ChannelTypeString);

  FRedwoodChatIdentity Sender;
  Message->TryGetStringField(TEXT("senderId"), Sender.PlayerId);
  Message->TryGetStringField(TEXT("senderName"), Sender.Nickname);

  FString Body;
  Message->TryGetStringField(TEXT("body"), Body);

  const FDateTime Timestamp = ParseTimestamp(Message);

  // A one-to-one message is reported by the space it arrived in, which is what
  // tells the game whether it came from an account or a character.
  if (Type == ERedwoodChatRoomType::Direct) {
    if (bCharacterSpace) {
      OnCharacterPrivateChatReceived.Broadcast(Sender, Timestamp, Body);
    } else {
      OnPlayerPrivateChatReceived.Broadcast(Sender, Timestamp, Body);
    }
    return;
  }

  FRedwoodChatRoomIdentity Room;
  Message->TryGetStringField(TEXT("channelKey"), Room.RedwoodId);
  Room.Type = Type;
  Room.CompleteRoomId =
    FString::Printf(TEXT("%s|%s"), *ChannelTypeString, *Room.RedwoodId);

  // Nearby carries where the sender was standing, stamped by the game server
  // rather than claimed by their client.
  FVector Location = FVector::ZeroVector;
  const TSharedPtr<FJsonObject> *Position;
  if (Message->TryGetObjectField(TEXT("position"), Position)) {
    (*Position)->TryGetNumberField(TEXT("x"), Location.X);
    (*Position)->TryGetNumberField(TEXT("y"), Location.Y);
    (*Position)->TryGetNumberField(TEXT("z"), Location.Z);
  }

  OnRoomChatReceived.Broadcast(Room, Sender, Timestamp, Body, Location);
}

void URedwoodClientChatSubsystem::JoinRoom(
  ERedwoodChatRoomType Type, FString Id
) {
  if (Type == ERedwoodChatRoomType::Custom) {
    UE_LOG(
      LogRedwoodChat,
      Error,
      TEXT("Use JoinCustomRoom for custom rooms; they are joined by name.")
    );
    return;
  }

  TSharedPtr<FSocketIONative> Connection = ConnectionFor(Type, false);
  if (!Connection.IsValid() || !Connection->bIsConnected) {
    UE_LOG(LogRedwoodChat, Error, TEXT("Not connected for that channel."));
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeRequest(IsCharacterSpace(Type));
  Payload->SetStringField(TEXT("channelType"), SerializeRoomType(Type));
  Payload->SetStringField(TEXT("channelKey"), Id);

  // Subscribing is a no-op that succeeds on channels that are always
  // delivered, so a caller does not have to know which kind this is.
  Connection->Emit(
    TEXT("chat:subscribe"),
    Payload,
    [this, Type, Id](auto Response) {
      const FString Error = ErrorOf(Response[0]->AsObject());
      if (!Error.IsEmpty()) {
        UE_LOG(LogRedwoodChat, Error, TEXT("Failed to join: %s"), *Error);
        return;
      }

      FRedwoodChatRoomIdentity Room;
      Room.Type = Type;
      Room.RedwoodId = Id;
      Room.CompleteRoomId =
        FString::Printf(TEXT("%s|%s"), *SerializeRoomType(Type), *Id);

      OnJoinPrivateRoom.Broadcast(Room);
    }
  );
}

void URedwoodClientChatSubsystem::JoinCustomRoom(
  FString Id, FString Password, bool bJoinAsCharacter
) {
  TSharedPtr<FSocketIONative> Connection =
    ConnectionFor(ERedwoodChatRoomType::Custom, bJoinAsCharacter);
  if (!Connection.IsValid() || !Connection->bIsConnected) {
    UE_LOG(LogRedwoodChat, Error, TEXT("Not connected for that channel."));
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeRequest(bJoinAsCharacter);
  Payload->SetStringField(TEXT("name"), Id);
  Payload->SetStringField(TEXT("joinCode"), Password);

  Connection->Emit(
    TEXT("chat:room:join"),
    Payload,
    [this, Id, bJoinAsCharacter](auto Response) {
      const TSharedPtr<FJsonObject> Object = Response[0]->AsObject();
      const FString Error = ErrorOf(Object);
      if (!Error.IsEmpty()) {
        UE_LOG(LogRedwoodChat, Error, TEXT("Failed to join room: %s"), *Error);
        return;
      }

      FString RoomId;
      Object->TryGetStringField(TEXT("channelKey"), RoomId);

      // Remembered so later sends and leaves go back to the same space; the
      // same name can exist as both an account room and a realm room.
      CustomRoomUsesCharacter.Add(RoomId, bJoinAsCharacter);

      FRedwoodChatRoomIdentity Room;
      Room.Type = ERedwoodChatRoomType::Custom;
      Room.RedwoodId = RoomId;
      Room.Name = Id;
      Room.CompleteRoomId = FString::Printf(TEXT("custom|%s"), *RoomId);

      OnJoinPrivateRoom.Broadcast(Room);
    }
  );
}

void URedwoodClientChatSubsystem::LeaveRoom(
  ERedwoodChatRoomType Type, FString Id
) {
  const bool bCharacterSpace = Type == ERedwoodChatRoomType::Custom
    ? CustomRoomUsesCharacter.FindRef(Id)
    : IsCharacterSpace(Type);

  TSharedPtr<FSocketIONative> Connection = ConnectionFor(Type, bCharacterSpace);
  if (!Connection.IsValid() || !Connection->bIsConnected) {
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeRequest(bCharacterSpace);

  if (Type == ERedwoodChatRoomType::Custom) {
    // Leaving a room is a membership change, not merely tuning out.
    Payload->SetStringField(TEXT("channelKey"), Id);
    Connection->Emit(TEXT("chat:room:leave"), Payload);
    CustomRoomUsesCharacter.Remove(Id);
    return;
  }

  Payload->SetStringField(TEXT("channelType"), SerializeRoomType(Type));
  Payload->SetStringField(TEXT("channelKey"), Id);
  Connection->Emit(TEXT("chat:unsubscribe"), Payload);
}

void URedwoodClientChatSubsystem::SendMessageToRoom(
  ERedwoodChatRoomType Type, FString Id, const FString &Message
) {
  if (Type == ERedwoodChatRoomType::Nearby) {
    UE_LOG(
      LogRedwoodChat,
      Error,
      TEXT("Nearby messages are sent by the game server, not the client. See "
           "URedwoodServerGameSubsystem::SendNearbyChatMessage.")
    );
    return;
  }

  const bool bCharacterSpace = Type == ERedwoodChatRoomType::Custom
    ? CustomRoomUsesCharacter.FindRef(Id)
    : IsCharacterSpace(Type);

  TSharedPtr<FSocketIONative> Connection = ConnectionFor(Type, bCharacterSpace);
  if (!Connection.IsValid() || !Connection->bIsConnected) {
    UE_LOG(LogRedwoodChat, Error, TEXT("Not connected for that channel."));
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeRequest(bCharacterSpace);
  Payload->SetStringField(TEXT("channelType"), SerializeRoomType(Type));
  Payload->SetStringField(TEXT("channelKey"), Id);
  Payload->SetStringField(TEXT("body"), Message);

  Connection->Emit(TEXT("chat:send"), Payload, [](auto Response) {
    const FString Error = ErrorOf(Response[0]->AsObject());
    if (!Error.IsEmpty()) {
      UE_LOG(LogRedwoodChat, Error, TEXT("Failed to send: %s"), *Error);
    }
  });
}

void URedwoodClientChatSubsystem::SendMessageToPlayer(
  const FString &TargetPlayerId, const FString &Message
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeRequest(false);
  Payload->SetStringField(TEXT("channelType"), TEXT("direct"));
  Payload->SetStringField(TEXT("recipientId"), TargetPlayerId);
  Payload->SetStringField(TEXT("body"), Message);

  Director->Emit(TEXT("chat:send"), Payload, [](auto Response) {
    const FString Error = ErrorOf(Response[0]->AsObject());
    if (!Error.IsEmpty()) {
      UE_LOG(LogRedwoodChat, Error, TEXT("Failed to send: %s"), *Error);
    }
  });
}

void URedwoodClientChatSubsystem::SendMessageToCharacter(
  const FString &TargetCharacterId, const FString &Message
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    UE_LOG(LogRedwoodChat, Error, TEXT("Not connected to a realm."));
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeRequest(true);
  Payload->SetStringField(TEXT("channelType"), TEXT("direct"));
  Payload->SetStringField(TEXT("recipientId"), TargetCharacterId);
  Payload->SetStringField(TEXT("body"), Message);

  Realm->Emit(TEXT("chat:send"), Payload, [](auto Response) {
    const FString Error = ErrorOf(Response[0]->AsObject());
    if (!Error.IsEmpty()) {
      UE_LOG(LogRedwoodChat, Error, TEXT("Failed to send: %s"), *Error);
    }
  });
}

void URedwoodClientChatSubsystem::CreateCustomRoom(
  FString Id,
  FString Password,
  bool bCreateAsCharacter,
  FRedwoodChatRoomCreatedOutputDelegate OnOutput
) {
  TSharedPtr<FSocketIONative> Connection =
    ConnectionFor(ERedwoodChatRoomType::Custom, bCreateAsCharacter);
  if (!Connection.IsValid() || !Connection->bIsConnected) {
    OnOutput.ExecuteIfBound(TEXT("Not connected for that channel."), TEXT(""));
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeRequest(bCreateAsCharacter);
  Payload->SetStringField(TEXT("name"), Id);

  // A password means "anyone with the name and this code"; no password means
  // "anyone with the name", which is what the old system did.
  Payload->SetStringField(
    TEXT("joinPolicy"), Password.IsEmpty() ? TEXT("open") : TEXT("code")
  );
  if (!Password.IsEmpty()) {
    Payload->SetStringField(TEXT("joinCode"), Password);
  }

  Connection->Emit(
    TEXT("chat:room:create"),
    Payload,
    [this, bCreateAsCharacter, OnOutput](auto Response) {
      const TSharedPtr<FJsonObject> Object = Response[0]->AsObject();
      const FString Error = ErrorOf(Object);

      FString JoinCode;
      Object->TryGetStringField(TEXT("joinCode"), JoinCode);

      if (Error.IsEmpty()) {
        FString RoomId;
        Object->TryGetStringField(TEXT("channelKey"), RoomId);
        CustomRoomUsesCharacter.Add(RoomId, bCreateAsCharacter);
      }

      OnOutput.ExecuteIfBound(Error, JoinCode);
    }
  );
}

void URedwoodClientChatSubsystem::ListRooms(
  bool bAsCharacter, FRedwoodChatRoomListOutputDelegate OnOutput
) {
  TSharedPtr<FSocketIONative> Connection =
    ConnectionFor(ERedwoodChatRoomType::Custom, bAsCharacter);
  if (!Connection.IsValid() || !Connection->bIsConnected) {
    OnOutput.ExecuteIfBound(TEXT("Not connected for that channel."), {});
    return;
  }

  Connection->Emit(
    TEXT("chat:room:list"),
    MakeRequest(bAsCharacter),
    [this, bAsCharacter, OnOutput](auto Response) {
      const TSharedPtr<FJsonObject> Object = Response[0]->AsObject();
      const TArray<FRedwoodChatRoom> Rooms = ParseRooms(Object);

      for (const FRedwoodChatRoom &Room : Rooms) {
        CustomRoomUsesCharacter.Add(Room.RoomId, bAsCharacter);
      }

      OnOutput.ExecuteIfBound(ErrorOf(Object), Rooms);
    }
  );
}

void URedwoodClientChatSubsystem::ListRoomInvites(
  bool bAsCharacter, FRedwoodChatRoomListOutputDelegate OnOutput
) {
  TSharedPtr<FSocketIONative> Connection =
    ConnectionFor(ERedwoodChatRoomType::Custom, bAsCharacter);
  if (!Connection.IsValid() || !Connection->bIsConnected) {
    OnOutput.ExecuteIfBound(TEXT("Not connected for that channel."), {});
    return;
  }

  Connection->Emit(
    TEXT("chat:room:list-invites"),
    MakeRequest(bAsCharacter),
    [OnOutput](auto Response) {
      const TSharedPtr<FJsonObject> Object = Response[0]->AsObject();
      OnOutput.ExecuteIfBound(ErrorOf(Object), ParseRooms(Object));
    }
  );
}

void URedwoodClientChatSubsystem::RespondToRoomInvite(
  FString RoomId, bool bAsCharacter, bool bAccept
) {
  TSharedPtr<FSocketIONative> Connection =
    ConnectionFor(ERedwoodChatRoomType::Custom, bAsCharacter);
  if (!Connection.IsValid() || !Connection->bIsConnected) {
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeRequest(bAsCharacter);
  Payload->SetStringField(TEXT("channelKey"), RoomId);
  Payload->SetBoolField(TEXT("accept"), bAccept);

  Connection->Emit(
    TEXT("chat:room:respond-to-invite"),
    Payload,
    [this, RoomId, bAsCharacter, bAccept](auto Response) {
      if (bAccept && ErrorOf(Response[0]->AsObject()).IsEmpty()) {
        CustomRoomUsesCharacter.Add(RoomId, bAsCharacter);
      }
    }
  );
}

void URedwoodClientChatSubsystem::InviteToRoom(
  FString RoomId, bool bAsCharacter, FString MemberId
) {
  TSharedPtr<FSocketIONative> Connection =
    ConnectionFor(ERedwoodChatRoomType::Custom, bAsCharacter);
  if (!Connection.IsValid() || !Connection->bIsConnected) {
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeRequest(bAsCharacter);
  Payload->SetStringField(TEXT("channelKey"), RoomId);
  Payload->SetStringField(TEXT("memberId"), MemberId);

  Connection->Emit(TEXT("chat:room:invite"), Payload, [](auto Response) {
    const FString Error = ErrorOf(Response[0]->AsObject());
    if (!Error.IsEmpty()) {
      UE_LOG(LogRedwoodChat, Error, TEXT("Failed to invite: %s"), *Error);
    }
  });
}

void URedwoodClientChatSubsystem::GetHistory(
  ERedwoodChatRoomType Type,
  FString Id,
  FString Before,
  FRedwoodChatHistoryOutputDelegate OnOutput
) {
  const bool bCharacterSpace = Type == ERedwoodChatRoomType::Custom
    ? CustomRoomUsesCharacter.FindRef(Id)
    : IsCharacterSpace(Type);

  TSharedPtr<FSocketIONative> Connection = ConnectionFor(Type, bCharacterSpace);
  if (!Connection.IsValid() || !Connection->bIsConnected) {
    OnOutput.ExecuteIfBound(TEXT("Not connected for that channel."), {});
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeRequest(bCharacterSpace);
  Payload->SetStringField(TEXT("channelType"), SerializeRoomType(Type));
  Payload->SetStringField(TEXT("channelKey"), Id);
  if (!Before.IsEmpty()) {
    Payload->SetStringField(TEXT("before"), Before);
  }

  Connection->Emit(TEXT("chat:history"), Payload, [OnOutput](auto Response) {
    const TSharedPtr<FJsonObject> Object = Response[0]->AsObject();

    TArray<FRedwoodChatMessage> Messages;
    const TArray<TSharedPtr<FJsonValue>> *Values;
    if (Object->TryGetArrayField(TEXT("messages"), Values)) {
      for (const TSharedPtr<FJsonValue> &Value : *Values) {
        const TSharedPtr<FJsonObject> Entry = Value->AsObject();
        if (!Entry.IsValid()) {
          continue;
        }

        FRedwoodChatMessage Message;
        Entry->TryGetStringField(TEXT("messageId"), Message.MessageId);
        Entry->TryGetStringField(TEXT("senderId"), Message.Sender.PlayerId);
        Entry->TryGetStringField(TEXT("senderName"), Message.Sender.Nickname);
        Entry->TryGetStringField(TEXT("body"), Message.Message);
        Message.Timestamp = ParseTimestamp(Entry);

        FString ChannelTypeString;
        Entry->TryGetStringField(TEXT("channelType"), ChannelTypeString);
        Entry->TryGetStringField(TEXT("channelKey"), Message.Room.RedwoodId);
        Message.Room.Type = ParseRoomType(ChannelTypeString);
        Message.Room.CompleteRoomId = FString::Printf(
          TEXT("%s|%s"), *ChannelTypeString, *Message.Room.RedwoodId
        );

        Messages.Add(Message);
      }
    }

    OnOutput.ExecuteIfBound(ErrorOf(Object), Messages);
  });
}

void URedwoodClientChatSubsystem::GetUnreadSummary(
  FRedwoodChatUnreadOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    OnOutput.ExecuteIfBound(TEXT("Not connected to the Director."), {});
    return;
  }

  // Unread state lives on whichever connection owns the channel, so a full
  // picture means asking both and merging. The realm half is skipped when the
  // player is not in one, which is correct rather than an error: they have no
  // character, so nothing there is addressed to them.
  TSharedPtr<TArray<FRedwoodChatUnreadChannel>> Merged =
    MakeShared<TArray<FRedwoodChatUnreadChannel>>();

  const bool bAskRealm = Realm.IsValid() && Realm->bIsConnected;

  auto Collect = [](const TSharedPtr<FJsonObject> &Object,
                    TArray<FRedwoodChatUnreadChannel> &Out) {
    const TArray<TSharedPtr<FJsonValue>> *Values;
    if (!Object->TryGetArrayField(TEXT("channels"), Values)) {
      return;
    }

    for (const TSharedPtr<FJsonValue> &Value : *Values) {
      const TSharedPtr<FJsonObject> Entry = Value->AsObject();
      if (!Entry.IsValid()) {
        continue;
      }

      FRedwoodChatUnreadChannel Channel;
      FString ChannelTypeString;
      Entry->TryGetStringField(TEXT("channelType"), ChannelTypeString);
      Entry->TryGetStringField(TEXT("channelKey"), Channel.Room.RedwoodId);
      Channel.Room.Type = ParseRoomType(ChannelTypeString);
      Channel.Room.CompleteRoomId = FString::Printf(
        TEXT("%s|%s"), *ChannelTypeString, *Channel.Room.RedwoodId
      );

      Entry->TryGetStringField(TEXT("displayName"), Channel.DisplayName);
      Entry->TryGetNumberField(TEXT("unreadCount"), Channel.UnreadCount);

      const TSharedPtr<FJsonObject> *Oldest;
      if (Entry->TryGetObjectField(TEXT("oldestUnread"), Oldest)) {
        (*Oldest)->TryGetStringField(
          TEXT("messageId"), Channel.OldestUnreadMessageId
        );
        (*Oldest)->TryGetStringField(TEXT("body"), Channel.OldestUnreadBody);
      }

      Out.Add(Channel);
    }
  };

  Director->Emit(
    TEXT("chat:unread-summary"),
    MakeRequest(false),
    [this, Merged, bAskRealm, Collect, OnOutput](auto Response) {
      const TSharedPtr<FJsonObject> Object = Response[0]->AsObject();
      const FString Error = ErrorOf(Object);
      Collect(Object, *Merged);

      if (!bAskRealm) {
        OnOutput.ExecuteIfBound(Error, *Merged);
        return;
      }

      Realm->Emit(
        TEXT("chat:unread-summary"),
        MakeRequest(true),
        [Merged, Collect, OnOutput](auto RealmResponse) {
          const TSharedPtr<FJsonObject> RealmObject =
            RealmResponse[0]->AsObject();
          Collect(RealmObject, *Merged);
          OnOutput.ExecuteIfBound(ErrorOf(RealmObject), *Merged);
        }
      );
    }
  );
}

void URedwoodClientChatSubsystem::MarkRead(
  ERedwoodChatRoomType Type, FString Id, FString UpToMessageId
) {
  const bool bCharacterSpace = Type == ERedwoodChatRoomType::Custom
    ? CustomRoomUsesCharacter.FindRef(Id)
    : IsCharacterSpace(Type);

  TSharedPtr<FSocketIONative> Connection = ConnectionFor(Type, bCharacterSpace);
  if (!Connection.IsValid() || !Connection->bIsConnected) {
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeRequest(bCharacterSpace);
  Payload->SetStringField(TEXT("channelType"), SerializeRoomType(Type));
  Payload->SetStringField(TEXT("channelKey"), Id);
  Payload->SetStringField(TEXT("upToMessageId"), UpToMessageId);

  Connection->Emit(TEXT("chat:mark-read"), Payload);
}

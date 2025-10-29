// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodChatClientSubsystem.h"
#include "RedwoodChatSettings.h"
#include "RedwoodClientGameSubsystem.h"
#include "RedwoodClientInterface.h"
#include "XmppModule.h"

void URedwoodClientChatSubsystem::Initialize(
  FSubsystemCollectionBase &Collection
) {
  Super::Initialize(Collection);
}

void URedwoodClientChatSubsystem::Deinitialize() {
  Super::Deinitialize();

  FRedwoodXmppModule &Module =
    FModuleManager::GetModuleChecked<FRedwoodXmppModule>("RedwoodXMPP");

  Module.Deinit();
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

  if (GameSubsystem) {
    URedwoodClientInterface *ClientInterface =
      GameSubsystem->GetClientInterface();

    if (ClientInterface) {
      if (!ClientInterface->IsDirectorConnected()) {
        OnOutput.ExecuteIfBound(TEXT("Not connected to the Director."));
        return;
      }

      Director = ClientInterface->GetDirectorConnection();
      PlayerId = ClientInterface->GetPlayerId();
      Nickname = ClientInterface->GetNickname();

      TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
      Payload->SetStringField(TEXT("playerId"), PlayerId);

      Director->Emit(
        TEXT("player:get-text-chat-credentials"),
        Payload,
        [this, OnOutput](auto Response) {
          TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

          FString Error = MessageObject->GetStringField(TEXT("error"));

          if (!Error.IsEmpty()) {
            OnOutput.ExecuteIfBound(Error);
            return;
          }

          XmppPassword = MessageObject->GetStringField(TEXT("xmppPassword"));

          FXmppServer XmppServer;
          XmppServer.ServerAddr = URedwoodChatSettings::GetXmppServerUri();
          XmppServer.Domain = XmppServer.ServerAddr;

          FRedwoodXmppModule &Module =
            FModuleManager::GetModuleChecked<FRedwoodXmppModule>("RedwoodXMPP");

          if (!Module.IsXmppEnabled()) {
            OnOutput.ExecuteIfBound(
              TEXT("XMPP is not enabled in the UE XMPP module.")
            );
            return;
          }

          Module.Init();

          XmppConnection = Module.CreateConnection(PlayerId).ToSharedPtr();

          XmppConnection->OnLoginComplete().AddLambda(
            [this, OnOutput](
              const FXmppUserJid &InUserJid,
              bool bWasSuccess,
              const FString &Error
            ) {
              if (bWasSuccess) {
                UserJid = InUserJid;
                bInitialized = true;
                OnOutput.ExecuteIfBound(TEXT(""));
                InitHandlers();

                FXmppUserPresence Presence;
                Presence.bIsAvailable = true;
                Presence.Status = EXmppPresenceStatus::Online;
                XmppConnection->Presence()->UpdatePresence(Presence);
              } else {
                OnOutput.ExecuteIfBound(Error);
              }

              XmppConnection->OnLoginComplete().Clear();
            }
          );

          XmppConnection->SetServer(XmppServer);
          XmppConnection->Login(PlayerId, XmppPassword);

          // poll to see if we logged in
        }
      );
    } else {
      OnOutput.ExecuteIfBound(TEXT("Redwood Client Interface not found."));
    }
  } else {
    OnOutput.ExecuteIfBound(TEXT("Redwood Client Game Subsystem not found."));
  }
}

bool URedwoodClientChatSubsystem::IsConnected() {
  return XmppConnection->GetLoginStatus() == EXmppLoginStatus::LoggedIn;
}

void URedwoodClientChatSubsystem::InitHandlers() {
  if (XmppConnection->PrivateChat().IsValid()) {
    XmppConnection->PrivateChat()->OnReceiveChat().AddUObject(
      this, &URedwoodClientChatSubsystem::HandlePrivateChatReceiveMessage
    );
  }

  if (XmppConnection->MultiUserChat().IsValid()) {
    XmppConnection->MultiUserChat()->OnRoomChatReceived().AddUObject(
      this, &URedwoodClientChatSubsystem::HandleRoomChatReceived
    );
    XmppConnection->MultiUserChat()->OnJoinPrivateRoom().AddUObject(
      this, &URedwoodClientChatSubsystem::HandleJoinPrivateRoom
    );
  }
}

void URedwoodClientChatSubsystem::HandlePrivateChatReceiveMessage(
  const TSharedRef<IXmppConnection> &Connection,
  const FXmppUserJid &InUserJid,
  const TSharedRef<FXmppChatMessage> &Message
) {
  FRedwoodChatIdentity SenderIdentity;
  SenderIdentity.PlayerId = InUserJid.Id;
  // SenderIdentity.Nickname = InUserJid.Nickname; // TODO

  FString Body = Message->Body;
  if (Body.IsEmpty()) {
    return;
  }

  TSharedPtr<FJsonObject> MessageObject;
  TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
  if (!FJsonSerializer::Deserialize(Reader, MessageObject)) {
    return;
  }

  if (!MessageObject->TryGetStringField(TEXT("nickname"), SenderIdentity.Nickname) || SenderIdentity.Nickname.IsEmpty()) {
    return;
  }

  FString MessageText;
  if (!MessageObject->TryGetStringField(TEXT("message"), MessageText) || MessageText.IsEmpty()) {
    return;
  }

  OnPrivateChatReceived.Broadcast(
    SenderIdentity, Message->Timestamp, MessageText
  );
}

void URedwoodClientChatSubsystem::HandleJoinPrivateRoom(
  const TSharedRef<IXmppConnection> &Connection,
  bool bSuccess,
  const FString &RoomId,
  const FString &Error
) {
  if (bSuccess) {
    FString RoomTypeString;
    FString RoomIdString;
    RoomId.Split(TEXT("|"), &RoomTypeString, &RoomIdString);

    FRedwoodChatRoomIdentity RoomIdentity;
    RoomIdentity.CompleteRoomId = RoomId;
    RoomIdentity.Type =
      URedwoodClientChatSubsystem::ParseRoomType(RoomTypeString);
    RoomIdentity.RedwoodId = RoomIdString;
    // TODO: RoomIdentity.Name

    OnJoinPrivateRoom.Broadcast(RoomIdentity);
  } else {
    UE_LOG(
      LogRedwoodChat, Error, TEXT("Failed to join private room: %s"), *Error
    );
  }
}

void URedwoodClientChatSubsystem::HandleRoomChatReceived(
  const TSharedRef<IXmppConnection> &Connection,
  const FString &RoomId,
  const FXmppUserJid &InUserJid,
  const TSharedRef<FXmppChatMessage> &ChatMsg
) {
  FString RoomTypeString;
  FString RoomIdString;
  RoomId.Split(TEXT("|"), &RoomTypeString, &RoomIdString);

  FString Body = ChatMsg->Body;
  if (Body.IsEmpty()) {
    return;
  }

  TSharedPtr<FJsonObject> MessageObject;
  TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
  if (!FJsonSerializer::Deserialize(Reader, MessageObject)) {
    return;
  }

  FString SenderPlayerId;
  if (!MessageObject->TryGetStringField(TEXT("playerId"), SenderPlayerId) || SenderPlayerId.IsEmpty()) {
    return;
  }

  FString MessageText;
  if (!MessageObject->TryGetStringField(TEXT("message"), MessageText) || MessageText.IsEmpty()) {
    return;
  }

  bool bHasLocation = false;
  FVector Location;
  FString LocationString;
  if (MessageObject->TryGetStringField(TEXT("location"), LocationString) && !LocationString.IsEmpty()) {
    if (Location.InitFromString(LocationString)) {
      bHasLocation = true;
    }
  }

  FRedwoodChatRoomIdentity RoomIdentity;
  RoomIdentity.CompleteRoomId = bHasLocation ? TEXT("Nearby") : RoomId;
  RoomIdentity.Type = bHasLocation
    ? ERedwoodChatRoomType::Nearby
    : URedwoodClientChatSubsystem::ParseRoomType(RoomTypeString);
  RoomIdentity.RedwoodId = RoomIdString;
  // TODO: RoomIdentity.Name

  FRedwoodChatIdentity SenderIdentity;
  SenderIdentity.PlayerId = SenderPlayerId;
  SenderIdentity.Nickname = InUserJid.Resource;

  OnRoomChatReceived.Broadcast(
    RoomIdentity, SenderIdentity, ChatMsg->Timestamp, MessageText, Location
  );
}

void URedwoodClientChatSubsystem::JoinRoom(
  ERedwoodChatRoomType Type, FString Id
) {
  if (!IsConnected()) {
    // TODO log error
    return;
  }

  FString RoomTypeString = URedwoodClientChatSubsystem::SerializeRoomType(Type);
  FString RoomId = FString::Printf(TEXT("%s|%s"), *RoomTypeString, *Id);
  XmppConnection->MultiUserChat()->JoinPrivateRoom(RoomId, Nickname, FString());
}

void URedwoodClientChatSubsystem::LeaveRoom(
  ERedwoodChatRoomType Type, FString Id
) {
  if (!IsConnected()) {
    // TODO log error
    return;
  }

  FString RoomTypeString = URedwoodClientChatSubsystem::SerializeRoomType(Type);
  FString RoomId = FString::Printf(TEXT("%s|%s"), *RoomTypeString, *Id);
  XmppConnection->MultiUserChat()->ExitRoom(RoomId);
}

void URedwoodClientChatSubsystem::SendMessageToRoom(
  ERedwoodChatRoomType Type, FString Id, const FString &Message
) {
  if (!IsConnected()) {
    // TODO log error
    return;
  }

  TSharedPtr<FJsonObject> MessagePayload = MakeShareable(new FJsonObject);
  MessagePayload->SetStringField(TEXT("playerId"), PlayerId);
  MessagePayload->SetStringField(TEXT("message"), Message);

  FString JsonString;
  TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
  FJsonSerializer::Serialize(MessagePayload.ToSharedRef(), Writer);
  Writer->Close();

  FString RoomTypeString = URedwoodClientChatSubsystem::SerializeRoomType(Type);
  FString RoomId = FString::Printf(TEXT("%s|%s"), *RoomTypeString, *Id);
  XmppConnection->MultiUserChat()->SendChat(RoomId, JsonString, FString());
}

void URedwoodClientChatSubsystem::SendNearbyMessage(
  const FString &ShardId, const FString &Message, const FVector &Location
) {
  if (!IsConnected()) {
    // TODO log error
    return;
  }

  TSharedPtr<FJsonObject> MessagePayload = MakeShareable(new FJsonObject);
  MessagePayload->SetStringField(TEXT("playerId"), PlayerId);
  MessagePayload->SetStringField(TEXT("message"), Message);
  MessagePayload->SetStringField(TEXT("location"), Location.ToString());

  FString JsonString;
  TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
  FJsonSerializer::Serialize(MessagePayload.ToSharedRef(), Writer);
  Writer->Close();

  FString RoomId = FString::Printf(TEXT("shard|%s"), *ShardId);
  XmppConnection->MultiUserChat()->SendChat(RoomId, JsonString, FString());
}

void URedwoodClientChatSubsystem::SendMessageToPlayer(
  const FString &TargetPlayerId, const FString &Message
) {
  if (!IsConnected()) {
    // TODO log error
    return;
  }

  FXmppUserJid RecipientJid;
  RecipientJid.Id = TargetPlayerId;
  RecipientJid.Domain = XmppConnection->GetServer().Domain;

  TSharedPtr<FJsonObject> MessagePayload = MakeShareable(new FJsonObject);
  MessagePayload->SetStringField(TEXT("nickname"), Nickname);
  MessagePayload->SetStringField(TEXT("message"), Message);

  FString JsonString;
  TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
  FJsonSerializer::Serialize(MessagePayload.ToSharedRef(), Writer);
  Writer->Close();

  XmppConnection->PrivateChat()->SendChat(RecipientJid, JsonString);
}
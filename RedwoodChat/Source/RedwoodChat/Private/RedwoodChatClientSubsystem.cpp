// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodChatClientSubsystem.h"
#include "RedwoodChatSettings.h"
#include "RedwoodClientGameSubsystem.h"
#include "RedwoodClientInterface.h"
#include "XmppModule.h"

void URedwoodChatClientSubsystem::Initialize(
  FSubsystemCollectionBase &Collection
) {
  Super::Initialize(Collection);
}

void URedwoodChatClientSubsystem::Deinitialize() {
  Super::Deinitialize();
}

void URedwoodChatClientSubsystem::InitializeChatConnection(
  FRedwoodErrorOutputDelegate OnOutput
) {
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

          FXmppModule &Module =
            FModuleManager::GetModuleChecked<FXmppModule>("XMPP");

          if (!Module.IsXmppEnabled()) {
            OnOutput.ExecuteIfBound(
              TEXT("XMPP is not enabled in the UE XMPP module.")
            );
            return;
          }

          XmppConnection = Module.CreateConnection(PlayerId).ToSharedPtr();

          XmppConnection->OnLoginComplete().AddLambda(
            [this, OnOutput](
              const FXmppUserJid &InUserJid,
              bool bWasSuccess,
              const FString &Error
            ) {
              if (bWasSuccess) {
                UserJid = InUserJid;
                OnOutput.ExecuteIfBound(TEXT(""));
                InitHandlers();
              } else {
                OnOutput.ExecuteIfBound(Error);
              }

              XmppConnection->OnLoginComplete().RemoveAll(this);
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

bool URedwoodChatClientSubsystem::IsConnected() {
  return XmppConnection->GetLoginStatus() == EXmppLoginStatus::LoggedIn;
}

void URedwoodChatClientSubsystem::InitHandlers() {
  if (XmppConnection->PrivateChat().IsValid()) {
    XmppConnection->PrivateChat()->OnReceiveChat().AddUObject(
      this, &URedwoodChatClientSubsystem::HandlePrivateChatReceiveMessage
    );
  }

  if (XmppConnection->MultiUserChat().IsValid()) {
    XmppConnection->MultiUserChat()->OnRoomChatReceived().AddUObject(
      this, &URedwoodChatClientSubsystem::HandleRoomChatReceived
    );
    XmppConnection->MultiUserChat()->OnJoinPrivateRoom().AddUObject(
      this, &URedwoodChatClientSubsystem::HandleJoinPrivateRoom
    );
  }
}

void URedwoodChatClientSubsystem::HandlePrivateChatReceiveMessage(
  const TSharedRef<IXmppConnection> &Connection,
  const FXmppUserJid &InUserJid,
  const TSharedRef<FXmppChatMessage> &Message
) {
  FRedwoodChatIdentity SenderIdentity;
  SenderIdentity.PlayerId = InUserJid.Id;
  // SenderIdentity.Nickname = InUserJid.Nickname; // TODO

  OnPrivateChatReceived.Broadcast(
    SenderIdentity, Message->Timestamp, Message->Body
  );
}

void URedwoodChatClientSubsystem::HandleJoinPrivateRoom(
  const TSharedRef<IXmppConnection> &Connection,
  bool bSuccess,
  const FString &RoomId,
  const FString &Error
) {
  if (bSuccess) {
    FString RoomTypeString;
    FString RoomIdString;
    RoomId.Split(TEXT(":"), &RoomTypeString, &RoomIdString);

    FRedwoodChatRoomIdentity RoomIdentity;
    RoomIdentity.CompleteRoomId = RoomId;
    RoomIdentity.Type =
      URedwoodChatClientSubsystem::ParseRoomType(RoomTypeString);
    RoomIdentity.RedwoodId = RoomIdString;
    // TODO: RoomIdentity.Name

    OnJoinPrivateRoom.Broadcast(RoomIdentity);
  } else {
    UE_LOG(
      LogRedwoodChat, Error, TEXT("Failed to join private room: %s"), *Error
    );
  }
}

void URedwoodChatClientSubsystem::HandleRoomChatReceived(
  const TSharedRef<IXmppConnection> &Connection,
  const FString &RoomId,
  const FXmppUserJid &InUserJid,
  const TSharedRef<FXmppChatMessage> &ChatMsg
) {
  FString RoomTypeString;
  FString RoomIdString;
  RoomId.Split(TEXT(":"), &RoomTypeString, &RoomIdString);

  FRedwoodChatRoomIdentity RoomIdentity;
  RoomIdentity.CompleteRoomId = RoomId;
  RoomIdentity.Type =
    URedwoodChatClientSubsystem::ParseRoomType(RoomTypeString);
  RoomIdentity.RedwoodId = RoomIdString;
  // TODO: RoomIdentity.Name

  FRedwoodChatIdentity SenderIdentity;
  SenderIdentity.PlayerId = InUserJid.Id;
  // SenderIdentity.Nickname = InUserJid.Nickname; // TODO

  OnRoomChatReceived.Broadcast(
    RoomIdentity, SenderIdentity, ChatMsg->Timestamp, ChatMsg->Body
  );
}

void URedwoodChatClientSubsystem::JoinRoom(
  ERedwoodChatRoomType Type, FString Id
) {
  if (!IsConnected()) {
    // TODO log error
    return;
  }

  FString RoomTypeString = URedwoodChatClientSubsystem::SerializeRoomType(Type);
  FString RoomId = FString::Printf(TEXT("%s:%s"), *RoomTypeString, *Id);
  XmppConnection->MultiUserChat()->JoinPrivateRoom(RoomId, Nickname, FString());
}

void URedwoodChatClientSubsystem::SendMessageToRoom(
  ERedwoodChatRoomType Type, FString Id, const FString &Message
) {
  if (!IsConnected()) {
    // TODO log error
    return;
  }

  FString RoomTypeString = URedwoodChatClientSubsystem::SerializeRoomType(Type);
  FString RoomId = FString::Printf(TEXT("%s:%s"), *RoomTypeString, *Id);
  XmppConnection->MultiUserChat()->SendChat(RoomId, Message, FString());
}

void URedwoodChatClientSubsystem::SendMessageToPlayer(
  const FString &TargetPlayerId, const FString &Message
) {
  if (!IsConnected()) {
    // TODO log error
    return;
  }

  FXmppUserJid RecipientJid;
  RecipientJid.Id = TargetPlayerId;
  RecipientJid.Domain = XmppConnection->GetServer().Domain;

  XmppConnection->PrivateChat()->SendChat(RecipientJid, Message);
}
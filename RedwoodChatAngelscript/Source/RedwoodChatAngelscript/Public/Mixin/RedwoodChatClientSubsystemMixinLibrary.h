// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RedwoodAngelscriptForwarding.h"
#include "RedwoodChatClientSubsystem.h"

#include "RedwoodChatClientSubsystemMixinLibrary.generated.h"

/**
 * ScriptMixin library for URedwoodClientChatSubsystem.
 *
 * Most of the chat surface is already UFUNCTION upstream (JoinRoom,
 * JoinCustomRoom, LeaveRoom, SendMessageToRoom, SendMessageToPlayer,
 * SendMessageToCharacter, RespondToRoomInvite, InviteToRoom, MarkRead) and is
 * visible to script without help. Only the six callback-taking methods need
 * adapting.
 *
 * Note the arity difference: chat callbacks are (Error, Payload) pairs, where
 * the RedwoodCore calls carry an Error field inside a single output struct.
 * Hence RW_FWD2 rather than RW_FWD.
 *
 * The class is named URedwoodClientChatSubsystem even though its header is
 * RedwoodChatClientSubsystem.h.
 */
UCLASS(Meta = (ScriptMixin = "URedwoodClientChatSubsystem"))
class REDWOODCHATANGELSCRIPT_API URedwoodClientChatSubsystemScriptMixinLibrary
  : public UObject {
  GENERATED_BODY()

public:
  UFUNCTION(ScriptCallable)
  static void InitializeChatConnection(
    URedwoodClientChatSubsystem *Subsystem,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->InitializeChatConnection(
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  /**
   * Supplying a password makes the room joinable by name plus that code;
   * leaving it empty makes it joinable by name alone. The code actually in
   * force comes back through the delegate, because the server generates one
   * when none is supplied.
   */
  UFUNCTION(ScriptCallable)
  static void CreateCustomRoom(
    URedwoodClientChatSubsystem *Subsystem,
    const FString &Id,
    const FString &Password,
    bool bCreateAsCharacter,
    FRedwoodChatRoomCreatedDynamicDelegate OnOutput
  ) {
    Subsystem->CreateCustomRoom(
      Id,
      Password,
      bCreateAsCharacter,
      RW_FWD2(
        FRedwoodChatRoomCreatedOutputDelegate, FString, FString, OnOutput
      )
    );
  }

  /** Rooms the player or character belongs to. */
  UFUNCTION(ScriptCallable)
  static void ListRooms(
    URedwoodClientChatSubsystem *Subsystem,
    bool bAsCharacter,
    FRedwoodChatRoomListDynamicDelegate OnOutput
  ) {
    Subsystem->ListRooms(
      bAsCharacter,
      RW_FWD2(
        FRedwoodChatRoomListOutputDelegate,
        FString,
        TArray<FRedwoodChatRoom>,
        OnOutput
      )
    );
  }

  /** Invitations awaiting an answer. */
  UFUNCTION(ScriptCallable)
  static void ListRoomInvites(
    URedwoodClientChatSubsystem *Subsystem,
    bool bAsCharacter,
    FRedwoodChatRoomListDynamicDelegate OnOutput
  ) {
    Subsystem->ListRoomInvites(
      bAsCharacter,
      RW_FWD2(
        FRedwoodChatRoomListOutputDelegate,
        FString,
        TArray<FRedwoodChatRoom>,
        OnOutput
      )
    );
  }

  /**
   * Earlier messages in a channel, newest first. Pass the id of the oldest
   * message already held as Before to page further back, or leave it empty for
   * the most recent page.
   */
  UFUNCTION(ScriptCallable)
  static void GetHistory(
    URedwoodClientChatSubsystem *Subsystem,
    ERedwoodChatRoomType Type,
    const FString &Id,
    const FString &Before,
    FRedwoodChatHistoryDynamicDelegate OnOutput
  ) {
    Subsystem->GetHistory(
      Type,
      Id,
      Before,
      RW_FWD2(
        FRedwoodChatHistoryOutputDelegate,
        FString,
        TArray<FRedwoodChatMessage>,
        OnOutput
      )
    );
  }

  /** Channels with unseen messages. Ambient channels never appear. */
  UFUNCTION(ScriptCallable)
  static void GetUnreadSummary(
    URedwoodClientChatSubsystem *Subsystem,
    FRedwoodChatUnreadDynamicDelegate OnOutput
  ) {
    Subsystem->GetUnreadSummary(RW_FWD2(
      FRedwoodChatUnreadOutputDelegate,
      FString,
      TArray<FRedwoodChatUnreadChannel>,
      OnOutput
    ));
  }
};

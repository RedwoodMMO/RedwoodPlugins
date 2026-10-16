// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Types/RedwoodTypesChat.h"

#include "RedwoodChatAngelscriptCallbacks.generated.h"

/**
 * Single-cast dynamic twins of RedwoodChat's callback delegates, so script can
 * pass a callback inline as FRedwoodChatRoomListDynamicCallback(this, n"Fn").
 * See RedwoodAngelscriptCallbacks.h for why these exist; the RedwoodCore twins
 * used by chat (FRedwoodErrorOutputDynamicCallback) are declared there.
 *
 * Each parameter list matches its FRedwoodChat*DynamicDelegate twin exactly.
 */

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_TwoParams(
  FRedwoodChatHistoryDynamicCallback,
  const FString &,
  Error,
  const TArray<FRedwoodChatMessage> &,
  Messages
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_TwoParams(
  FRedwoodChatUnreadDynamicCallback,
  const FString &,
  Error,
  const TArray<FRedwoodChatUnreadChannel> &,
  Channels
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_TwoParams(
  FRedwoodChatRoomListDynamicCallback,
  const FString &,
  Error,
  const TArray<FRedwoodChatRoom> &,
  Rooms
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_TwoParams(
  FRedwoodChatRoomCreatedDynamicCallback,
  const FString &,
  Error,
  const FString &,
  JoinCode
);

// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Types/RedwoodTypes.h"

#include "RedwoodAngelscriptCallbacks.generated.h"

/**
 * Single-cast dynamic twins of RedwoodCore's callback delegates, for the
 * ScriptMixin libraries to take as parameters.
 *
 * Why these exist: RedwoodCore's FRedwood*DynamicDelegate types are multicast,
 * because the BlueprintAsync nodes expose them as BlueprintAssignable output
 * pins, and Angelscript gives multicast delegates no (Object, FunctionName)
 * constructor. A single-cast parameter lets script pass a callback inline:
 *
 *   URedwoodClientGameSubsystem::Get().AttemptAutoLogin(
 *     FRedwoodAuthUpdateDynamicCallback(this, n"HandleAuthUpdate")
 *   );
 *
 * They live here rather than in RedwoodCore so that projects without
 * Angelscript never see them.
 *
 * Each parameter list matches its FRedwood*DynamicDelegate twin exactly, so a
 * handler UFUNCTION written for one binds to the other unchanged. Angelscript
 * checks the handler's signature when the callback is constructed.
 */

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE(FRedwoodDynamicCallback);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodErrorOutputDynamicCallback, FString, Error
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodSocketConnectedDynamicCallback, FRedwoodSocketConnected, Details
);

// Auth

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodAuthUpdateDynamicCallback, FRedwoodAuthUpdate, Data
);

// Blobs and save games

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodGetBlobOutputDynamicCallback, FRedwoodGetBlobOutput, Data
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodGetSaveGameOutputDynamicCallback, FRedwoodGetSaveGameOutput, Data
);

// Characters and realm contacts

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodListCharactersOutputDynamicCallback,
  FRedwoodListCharactersOutput,
  Data
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodGetCharacterOutputDynamicCallback, FRedwoodGetCharacterOutput, Data
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodListRealmContactsOutputDynamicCallback,
  FRedwoodListRealmContactsOutput,
  Data
);

// Global data

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodGetGlobalDataOutputDynamicCallback, FRedwoodGetGlobalDataOutput, Data
);

// Guilds and alliances

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodListGuildsOutputDynamicCallback, FRedwoodListGuildsOutput, Data
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodGetGuildOutputDynamicCallback, FRedwoodGetGuildOutput, Data
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodListGuildMembersOutputDynamicCallback,
  FRedwoodListGuildMembersOutput,
  Data
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodCreateGuildOutputDynamicCallback, FRedwoodCreateGuildOutput, Data
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodListAlliancesOutputDynamicCallback, FRedwoodListAlliancesOutput, Data
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodCreateAllianceOutputDynamicCallback,
  FRedwoodCreateAllianceOutput,
  Data
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodListAllianceGuildsOutputDynamicCallback,
  FRedwoodListAllianceGuildsOutput,
  Data
);

// Parties

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodGetPartyOutputDynamicCallback, FRedwoodGetPartyOutput, Data
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodListPartyInvitesOutputDynamicCallback,
  FRedwoodListPartyInvitesOutput,
  Data
);

// Persistent items

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodPersistentItemsOutputDynamicCallback,
  FRedwoodPersistentItemsOutput,
  Output
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodPersistentItemsTreeOutputDynamicCallback,
  FRedwoodPersistentItemsTreeOutput,
  Output
);

// Players

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodListPlayersOutputDynamicCallback, FRedwoodListPlayersOutput, Data
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodPlayerOutputDynamicCallback, FRedwoodPlayerOutput, Data
);

// Realms

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodListRealmsOutputDynamicCallback, FRedwoodListRealmsOutput, Output
);

// Servers and proxies

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodListProxiesOutputDynamicCallback, FRedwoodListProxiesOutput, Data
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodCreateProxyOutputDynamicCallback, FRedwoodCreateProxyOutput, Data
);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodJoinServerOutputDynamicCallback, FRedwoodJoinServerOutput, Data
);

// Ticketing

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(
  FRedwoodTicketingUpdateDynamicCallback, FRedwoodTicketingUpdate, Update
);

// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RedwoodAngelscriptForwarding.h"
#include "RedwoodServerGameSubsystem.h"

#include "RedwoodServerGameSubsystemMixinLibrary.generated.h"

class USaveGame;

/**
 * ScriptMixin library exposing URedwoodServerGameSubsystem's delegate-callback
 * API to Angelscript.
 *
 * Methods already marked UFUNCTION in RedwoodCore (CallExecCommandOnAllClients,
 * TravelPlayerToZoneTransform, TravelPlayerToZoneSpawnName,
 * FlushPlayerCharacterData, RequestEngineExit) are visible to script already
 * and are deliberately not duplicated here, as are the BlueprintReadOnly
 * properties (RequestId, RealmName, ProxyId, InstanceId, MapId, ModeId,
 * Channel, ZoneName, ShardName, ParentProxyId, MaxPlayers, TrackedParties).
 *
 * SendNearbyChatMessage takes no callback, so it is a plain forward: it is here
 * only because it is not a UFUNCTION upstream.
 */
UCLASS(Meta = (ScriptMixin = "URedwoodServerGameSubsystem"))
class REDWOODANGELSCRIPT_API URedwoodServerGameSubsystemScriptMixinLibrary
  : public UObject {
  GENERATED_BODY()

public:
  // ---------------------------------------------------------------------
  // Lifecycle and persistence
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void FlushSync(URedwoodServerGameSubsystem *Subsystem) {
    Subsystem->FlushSync();
  }

  UFUNCTION(ScriptCallable)
  static void FlushPersistence(URedwoodServerGameSubsystem *Subsystem) {
    Subsystem->FlushPersistence();
  }

  UFUNCTION(ScriptCallable)
  static void FlushZoneData(URedwoodServerGameSubsystem *Subsystem) {
    Subsystem->FlushZoneData();
  }

  UFUNCTION(ScriptCallable)
  static void InitialDataLoad(
    URedwoodServerGameSubsystem *Subsystem,
    FRedwoodDynamicDelegate OnComplete
  ) {
    Subsystem->InitialDataLoad(RW_FWD_VOID(OnComplete));
  }

  // ---------------------------------------------------------------------
  // Blobs and save games
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void PutBlob(
    URedwoodServerGameSubsystem *Subsystem,
    const FString &Key,
    const TArray<uint8> &Value,
    FRedwoodErrorOutputDynamicDelegate OnComplete
  ) {
    Subsystem->PutBlob(
      Key, Value, RW_FWD(FRedwoodErrorOutputDelegate, FString, OnComplete)
    );
  }

  UFUNCTION(ScriptCallable)
  static void GetBlob(
    URedwoodServerGameSubsystem *Subsystem,
    const FString &Key,
    FRedwoodGetBlobOutputDynamicDelegate OnComplete
  ) {
    Subsystem->GetBlob(
      Key,
      RW_FWD(FRedwoodGetBlobOutputDelegate, FRedwoodGetBlobOutput, OnComplete)
    );
  }

  UFUNCTION(ScriptCallable)
  static void PutSaveGame(
    URedwoodServerGameSubsystem *Subsystem,
    const FString &Key,
    USaveGame *Value,
    FRedwoodErrorOutputDynamicDelegate OnComplete
  ) {
    Subsystem->PutSaveGame(
      Key, Value, RW_FWD(FRedwoodErrorOutputDelegate, FString, OnComplete)
    );
  }

  UFUNCTION(ScriptCallable)
  static void GetSaveGame(
    URedwoodServerGameSubsystem *Subsystem,
    const FString &Key,
    FRedwoodGetSaveGameOutputDynamicDelegate OnComplete
  ) {
    Subsystem->GetSaveGame(
      Key,
      RW_FWD(
        FRedwoodGetSaveGameOutputDelegate,
        FRedwoodGetSaveGameOutput,
        OnComplete
      )
    );
  }

  // ---------------------------------------------------------------------
  // Persistent items (the inventory workflow)
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void FetchPersistentItems(
    URedwoodServerGameSubsystem *Subsystem,
    const FRedwoodPersistentItemsFilter &Filter,
    FRedwoodPersistentItemsTreeOutputDynamicDelegate OnOutput
  ) {
    Subsystem->FetchPersistentItems(
      Filter,
      RW_FWD(
        FRedwoodPersistentItemsTreeOutputDelegate,
        FRedwoodPersistentItemsTreeOutput,
        OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void FetchCharacterPersistentItems(
    URedwoodServerGameSubsystem *Subsystem,
    const FString &CharacterId,
    FRedwoodPersistentItemsTreeOutputDynamicDelegate OnOutput
  ) {
    Subsystem->FetchCharacterPersistentItems(
      CharacterId,
      RW_FWD(
        FRedwoodPersistentItemsTreeOutputDelegate,
        FRedwoodPersistentItemsTreeOutput,
        OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void SavePersistentItems(
    URedwoodServerGameSubsystem *Subsystem,
    const TArray<FRedwoodSavePersistentItem> &Items,
    FRedwoodPersistentItemsOutputDynamicDelegate OnOutput
  ) {
    Subsystem->SavePersistentItems(
      Items,
      RW_FWD(
        FRedwoodPersistentItemsOutputDelegate,
        FRedwoodPersistentItemsOutput,
        OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void MovePersistentItemsToParent(
    URedwoodServerGameSubsystem *Subsystem,
    const TArray<FString> &ItemIds,
    const FString &NewParentId,
    FRedwoodPersistentItemsOutputDynamicDelegate OnOutput
  ) {
    Subsystem->MovePersistentItemsToParent(
      ItemIds,
      NewParentId,
      RW_FWD(
        FRedwoodPersistentItemsOutputDelegate,
        FRedwoodPersistentItemsOutput,
        OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void MovePersistentItemsToCharacter(
    URedwoodServerGameSubsystem *Subsystem,
    const TArray<FString> &ItemIds,
    const FString &NewOwnerCharacterId,
    FRedwoodPersistentItemsOutputDynamicDelegate OnOutput
  ) {
    Subsystem->MovePersistentItemsToCharacter(
      ItemIds,
      NewOwnerCharacterId,
      RW_FWD(
        FRedwoodPersistentItemsOutputDelegate,
        FRedwoodPersistentItemsOutput,
        OnOutput
      )
    );
  }

  /**
   * InZoneName defaults to this server's zone when empty. The caller still has
   * to spawn the actor, with its URedwoodSyncComponent RedwoodId set to the
   * item id so the sync flow updates the same row.
   */
  UFUNCTION(ScriptCallable)
  static void MovePersistentItemsToWorld(
    URedwoodServerGameSubsystem *Subsystem,
    const TArray<FString> &ItemIds,
    const FTransform &Transform,
    FRedwoodPersistentItemsOutputDynamicDelegate OnOutput,
    const FString &InZoneName
  ) {
    Subsystem->MovePersistentItemsToWorld(
      ItemIds,
      Transform,
      RW_FWD(
        FRedwoodPersistentItemsOutputDelegate,
        FRedwoodPersistentItemsOutput,
        OnOutput
      ),
      InZoneName
    );
  }

  UFUNCTION(ScriptCallable)
  static void DeletePersistentItems(
    URedwoodServerGameSubsystem *Subsystem,
    const TArray<FString> &ItemIds,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->DeletePersistentItems(
      ItemIds, RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  // ---------------------------------------------------------------------
  // Parties
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void GetPartyById(
    URedwoodServerGameSubsystem *Subsystem,
    const FString &PartyId,
    FRedwoodGetPartyOutputDynamicDelegate OnOutput
  ) {
    Subsystem->GetPartyById(
      PartyId,
      RW_FWD(FRedwoodGetPartyOutputDelegate, FRedwoodGetPartyOutput, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void GetPartyByPlayerId(
    URedwoodServerGameSubsystem *Subsystem,
    const FString &PlayerId,
    FRedwoodGetPartyOutputDynamicDelegate OnOutput
  ) {
    Subsystem->GetPartyByPlayerId(
      PlayerId,
      RW_FWD(FRedwoodGetPartyOutputDelegate, FRedwoodGetPartyOutput, OnOutput)
    );
  }

  // ---------------------------------------------------------------------
  // Chat
  // ---------------------------------------------------------------------

  /**
   * Nearby chat is the one channel a client cannot send, because it needs a
   * server-vouched position and a server-decided audience. Call this from a
   * Server RPC once the player has typed something.
   */
  UFUNCTION(ScriptCallable)
  static void SendNearbyChatMessage(
    URedwoodServerGameSubsystem *Subsystem,
    const FString &SenderCharacterId,
    const FString &Message,
    const FVector &Location,
    const TArray<FString> &RecipientCharacterIds
  ) {
    Subsystem->SendNearbyChatMessage(
      SenderCharacterId, Message, Location, RecipientCharacterIds
    );
  }
};

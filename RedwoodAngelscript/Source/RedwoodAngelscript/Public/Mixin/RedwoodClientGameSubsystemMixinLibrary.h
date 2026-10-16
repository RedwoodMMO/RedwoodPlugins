// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RedwoodAngelscriptForwarding.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodClientGameSubsystemMixinLibrary.generated.h"

class USIOJsonObject;

/**
 * ScriptMixin library exposing URedwoodClientGameSubsystem's delegate-callback
 * API to Angelscript. Methods already marked UFUNCTION in RedwoodCore
 * (IsLoggedIn, GetPlayerId, GetNickname, Logout, SetSelectedCharacter,
 * GetRegions, IsDirectorConnected, IsRealmConnected, GetCachedParty,
 * SendEmoteToParty, GetConnectionConsoleCommand,
 * CancelWaitingForAccountVerification) are visible to script already and are
 * deliberately not duplicated here.
 */
UCLASS(Meta = (ScriptMixin = "URedwoodClientGameSubsystem"))
class REDWOODANGELSCRIPT_API URedwoodClientGameSubsystemScriptMixinLibrary
  : public UObject {
  GENERATED_BODY()

public:
  // ---------------------------------------------------------------------
  // Connection
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void InitializeDirectorConnection(
    URedwoodClientGameSubsystem *Subsystem,
    FRedwoodSocketConnectedDynamicDelegate OnDirectorConnected
  ) {
    Subsystem->InitializeDirectorConnection(RW_FWD(
      FRedwoodSocketConnectedDelegate,
      FRedwoodSocketConnected,
      OnDirectorConnected
    ));
  }

  // ---------------------------------------------------------------------
  // Auth
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void Register(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &Username,
    const FString &Password,
    FRedwoodAuthUpdateDynamicDelegate OnUpdate
  ) {
    Subsystem->Register(
      Username,
      Password,
      RW_FWD(FRedwoodAuthUpdateDelegate, FRedwoodAuthUpdate, OnUpdate)
    );
  }

  UFUNCTION(ScriptCallable)
  static void AttemptAutoLogin(
    URedwoodClientGameSubsystem *Subsystem,
    FRedwoodAuthUpdateDynamicDelegate OnUpdate
  ) {
    Subsystem->AttemptAutoLogin(
      RW_FWD(FRedwoodAuthUpdateDelegate, FRedwoodAuthUpdate, OnUpdate)
    );
  }

  UFUNCTION(ScriptCallable)
  static void Login(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &Username,
    const FString &PasswordOrToken,
    const FString &Provider,
    bool bRememberMe,
    FRedwoodAuthUpdateDynamicDelegate OnUpdate
  ) {
    Subsystem->Login(
      Username,
      PasswordOrToken,
      Provider,
      bRememberMe,
      RW_FWD(FRedwoodAuthUpdateDelegate, FRedwoodAuthUpdate, OnUpdate)
    );
  }

  UFUNCTION(ScriptCallable)
  static void LoginWithDiscord(
    URedwoodClientGameSubsystem *Subsystem,
    bool bRememberMe,
    FRedwoodAuthUpdateDynamicDelegate OnUpdate
  ) {
    Subsystem->LoginWithDiscord(
      bRememberMe,
      RW_FWD(FRedwoodAuthUpdateDelegate, FRedwoodAuthUpdate, OnUpdate)
    );
  }

  UFUNCTION(ScriptCallable)
  static void LoginWithTwitch(
    URedwoodClientGameSubsystem *Subsystem,
    bool bRememberMe,
    FRedwoodAuthUpdateDynamicDelegate OnUpdate
  ) {
    Subsystem->LoginWithTwitch(
      bRememberMe,
      RW_FWD(FRedwoodAuthUpdateDelegate, FRedwoodAuthUpdate, OnUpdate)
    );
  }

  // ---------------------------------------------------------------------
  // Realms
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void ListRealms(
    URedwoodClientGameSubsystem *Subsystem,
    FRedwoodListRealmsOutputDynamicDelegate OnOutput
  ) {
    Subsystem->ListRealms(RW_FWD(
      FRedwoodListRealmsOutputDelegate, FRedwoodListRealmsOutput, OnOutput
    ));
  }

  UFUNCTION(ScriptCallable)
  static void InitializeRealmConnection(
    URedwoodClientGameSubsystem *Subsystem,
    FRedwoodRealm InRealm,
    FRedwoodSocketConnectedDynamicDelegate OnRealmConnected
  ) {
    Subsystem->InitializeRealmConnection(
      InRealm,
      RW_FWD(
        FRedwoodSocketConnectedDelegate,
        FRedwoodSocketConnected,
        OnRealmConnected
      )
    );
  }

  /**
   * Convenience for single-realm games and dev environments: calls ListRealms
   * and then InitializeRealmConnection on the first realm returned.
   */
  UFUNCTION(ScriptCallable)
  static void InitializeConnectionForFirstRealm(
    URedwoodClientGameSubsystem *Subsystem,
    FRedwoodSocketConnectedDynamicDelegate OnRealmConnected
  ) {
    Subsystem->InitializeConnectionForFirstRealm(RW_FWD(
      FRedwoodSocketConnectedDelegate,
      FRedwoodSocketConnected,
      OnRealmConnected
    ));
  }

  // ---------------------------------------------------------------------
  // Characters
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void ListCharacters(
    URedwoodClientGameSubsystem *Subsystem,
    FRedwoodListCharactersOutputDynamicDelegate OnOutput
  ) {
    Subsystem->ListCharacters(RW_FWD(
      FRedwoodListCharactersOutputDelegate,
      FRedwoodListCharactersOutput,
      OnOutput
    ));
  }

  UFUNCTION(ScriptCallable)
  static void ListArchivedCharacters(
    URedwoodClientGameSubsystem *Subsystem,
    FRedwoodListCharactersOutputDynamicDelegate OnOutput
  ) {
    Subsystem->ListArchivedCharacters(RW_FWD(
      FRedwoodListCharactersOutputDelegate,
      FRedwoodListCharactersOutput,
      OnOutput
    ));
  }

  UFUNCTION(ScriptCallable)
  static void CreateCharacter(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &Name,
    USIOJsonObject *CharacterCreatorData,
    FRedwoodGetCharacterOutputDynamicDelegate OnOutput
  ) {
    Subsystem->CreateCharacter(
      Name,
      CharacterCreatorData,
      RW_FWD(
        FRedwoodGetCharacterOutputDelegate,
        FRedwoodGetCharacterOutput,
        OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void GetCharacterData(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &CharacterIdOrName,
    FRedwoodGetCharacterOutputDynamicDelegate OnOutput
  ) {
    Subsystem->GetCharacterData(
      CharacterIdOrName,
      RW_FWD(
        FRedwoodGetCharacterOutputDelegate,
        FRedwoodGetCharacterOutput,
        OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void SetCharacterData(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &CharacterId,
    const FString &Name,
    USIOJsonObject *CharacterCreatorData,
    FRedwoodGetCharacterOutputDynamicDelegate OnOutput
  ) {
    Subsystem->SetCharacterData(
      CharacterId,
      Name,
      CharacterCreatorData,
      RW_FWD(
        FRedwoodGetCharacterOutputDelegate,
        FRedwoodGetCharacterOutput,
        OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void SetCharacterArchived(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &CharacterId,
    bool bArchived,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->SetCharacterArchived(
      CharacterId,
      bArchived,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  // ---------------------------------------------------------------------
  // Ticketing
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void JoinQueue(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &ProxyId,
    const FString &ZoneName,
    bool bTransferWholeParty,
    bool bFavorLastZone,
    FRedwoodTicketingUpdateDynamicDelegate OnUpdate
  ) {
    Subsystem->JoinQueue(
      ProxyId,
      ZoneName,
      bTransferWholeParty,
      bFavorLastZone,
      RW_FWD(
        FRedwoodTicketingUpdateDelegate, FRedwoodTicketingUpdate, OnUpdate
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void JoinMatchmaking(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &ProfileId,
    const TArray<FString> &InRegions,
    FRedwoodTicketingUpdateDynamicDelegate OnUpdate
  ) {
    Subsystem->JoinMatchmaking(
      ProfileId,
      InRegions,
      RW_FWD(
        FRedwoodTicketingUpdateDelegate, FRedwoodTicketingUpdate, OnUpdate
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void JoinCustom(
    URedwoodClientGameSubsystem *Subsystem,
    bool bTransferWholeParty,
    const TArray<FString> &InRegions,
    FRedwoodTicketingUpdateDynamicDelegate OnUpdate
  ) {
    Subsystem->JoinCustom(
      bTransferWholeParty,
      InRegions,
      RW_FWD(
        FRedwoodTicketingUpdateDelegate, FRedwoodTicketingUpdate, OnUpdate
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void LeaveTicketing(
    URedwoodClientGameSubsystem *Subsystem,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->LeaveTicketing(
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  // ---------------------------------------------------------------------
  // Parties
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void GetOrCreateParty(
    URedwoodClientGameSubsystem *Subsystem,
    bool bCreateIfNotInParty,
    FRedwoodGetPartyOutputDynamicDelegate OnOutput
  ) {
    Subsystem->GetOrCreateParty(
      bCreateIfNotInParty,
      RW_FWD(FRedwoodGetPartyOutputDelegate, FRedwoodGetPartyOutput, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void LeaveParty(
    URedwoodClientGameSubsystem *Subsystem,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->LeaveParty(
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void InviteToParty(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &TargetPlayerId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->InviteToParty(
      TargetPlayerId, RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void ListPartyInvites(
    URedwoodClientGameSubsystem *Subsystem,
    FRedwoodListPartyInvitesOutputDynamicDelegate OnOutput
  ) {
    Subsystem->ListPartyInvites(RW_FWD(
      FRedwoodListPartyInvitesOutputDelegate,
      FRedwoodListPartyInvitesOutput,
      OnOutput
    ));
  }

  UFUNCTION(ScriptCallable)
  static void RespondToPartyInvite(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &PartyId,
    bool bAccept,
    FRedwoodGetPartyOutputDynamicDelegate OnOutput
  ) {
    Subsystem->RespondToPartyInvite(
      PartyId,
      bAccept,
      RW_FWD(FRedwoodGetPartyOutputDelegate, FRedwoodGetPartyOutput, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void PromoteToPartyLeader(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &TargetPlayerId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->PromoteToPartyLeader(
      TargetPlayerId, RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void KickFromParty(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &TargetPlayerId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->KickFromParty(
      TargetPlayerId, RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void SetPartyData(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &LootType,
    USIOJsonObject *PartyData,
    FRedwoodGetPartyOutputDynamicDelegate OnOutput
  ) {
    Subsystem->SetPartyData(
      LootType,
      PartyData,
      RW_FWD(FRedwoodGetPartyOutputDelegate, FRedwoodGetPartyOutput, OnOutput)
    );
  }

  // ---------------------------------------------------------------------
  // Global data
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void GetDirectorGlobalData(
    URedwoodClientGameSubsystem *Subsystem,
    FRedwoodGetGlobalDataOutputDynamicDelegate OnOutput
  ) {
    Subsystem->GetDirectorGlobalData(RW_FWD(
      FRedwoodGetGlobalDataOutputDelegate,
      FRedwoodGetGlobalDataOutput,
      OnOutput
    ));
  }

  UFUNCTION(ScriptCallable)
  static void GetRealmGlobalData(
    URedwoodClientGameSubsystem *Subsystem,
    FRedwoodGetGlobalDataOutputDynamicDelegate OnOutput
  ) {
    Subsystem->GetRealmGlobalData(RW_FWD(
      FRedwoodGetGlobalDataOutputDelegate,
      FRedwoodGetGlobalDataOutput,
      OnOutput
    ));
  }

  // ---------------------------------------------------------------------
  // Player search
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void SearchForPlayers(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &UsernameOrNickname,
    bool bIncludePartialMatches,
    FRedwoodListPlayersOutputDynamicDelegate OnOutput
  ) {
    Subsystem->SearchForPlayers(
      UsernameOrNickname,
      bIncludePartialMatches,
      RW_FWD(
        FRedwoodListPlayersOutputDelegate, FRedwoodListPlayersOutput, OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void SearchForPlayerById(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &TargetPlayerId,
    FRedwoodPlayerOutputDynamicDelegate OnOutput
  ) {
    Subsystem->SearchForPlayerById(
      TargetPlayerId,
      RW_FWD(FRedwoodPlayerOutputDelegate, FRedwoodPlayerOutput, OnOutput)
    );
  }

  // ---------------------------------------------------------------------
  // Friends (account level, at the director)
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void ListFriends(
    URedwoodClientGameSubsystem *Subsystem,
    ERedwoodFriendListType Filter,
    FRedwoodListPlayersOutputDynamicDelegate OnOutput
  ) {
    Subsystem->ListFriends(
      Filter,
      RW_FWD(
        FRedwoodListPlayersOutputDelegate, FRedwoodListPlayersOutput, OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void RequestFriend(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &OtherPlayerId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->RequestFriend(
      OtherPlayerId, RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void RemoveFriend(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &OtherPlayerId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->RemoveFriend(
      OtherPlayerId, RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void RespondToFriendRequest(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &OtherPlayerId,
    bool bAccept,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->RespondToFriendRequest(
      OtherPlayerId,
      bAccept,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void SetPlayerBlocked(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &OtherPlayerId,
    bool bBlocked,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->SetPlayerBlocked(
      OtherPlayerId,
      bBlocked,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  // ---------------------------------------------------------------------
  // Realm contacts (character level, at the realm)
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void ListRealmContacts(
    URedwoodClientGameSubsystem *Subsystem,
    FRedwoodListRealmContactsOutputDynamicDelegate OnOutput
  ) {
    Subsystem->ListRealmContacts(RW_FWD(
      FRedwoodListRealmContactsOutputDelegate,
      FRedwoodListRealmContactsOutput,
      OnOutput
    ));
  }

  UFUNCTION(ScriptCallable)
  static void AddRealmContact(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &OtherCharacterId,
    bool bBlocked,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->AddRealmContact(
      OtherCharacterId,
      bBlocked,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void RemoveRealmContact(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &OtherCharacterId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->RemoveRealmContact(
      OtherCharacterId, RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  // ---------------------------------------------------------------------
  // Guilds
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void ListGuilds(
    URedwoodClientGameSubsystem *Subsystem,
    bool bOnlyPlayersGuilds,
    FRedwoodListGuildsOutputDynamicDelegate OnOutput
  ) {
    Subsystem->ListGuilds(
      bOnlyPlayersGuilds,
      RW_FWD(
        FRedwoodListGuildsOutputDelegate, FRedwoodListGuildsOutput, OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void SearchForGuilds(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &SearchText,
    bool bIncludePartialMatches,
    FRedwoodListGuildsOutputDynamicDelegate OnOutput
  ) {
    Subsystem->SearchForGuilds(
      SearchText,
      bIncludePartialMatches,
      RW_FWD(
        FRedwoodListGuildsOutputDelegate, FRedwoodListGuildsOutput, OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void GetGuild(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &GuildId,
    FRedwoodGetGuildOutputDynamicDelegate OnOutput
  ) {
    Subsystem->GetGuild(
      GuildId,
      RW_FWD(FRedwoodGetGuildOutputDelegate, FRedwoodGetGuildOutput, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void GetSelectedGuild(
    URedwoodClientGameSubsystem *Subsystem,
    FRedwoodGetGuildOutputDynamicDelegate OnOutput
  ) {
    Subsystem->GetSelectedGuild(
      RW_FWD(FRedwoodGetGuildOutputDelegate, FRedwoodGetGuildOutput, OnOutput)
    );
  }

  /**
   * The selected guild is replicated to URedwoodCharacterComponent::SelectedGuild,
   * which is what name plates read.
   */
  UFUNCTION(ScriptCallable)
  static void SetSelectedGuild(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &GuildId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->SetSelectedGuild(
      GuildId, RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void JoinGuild(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &GuildId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->JoinGuild(
      GuildId, RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void LeaveGuild(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &GuildId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->LeaveGuild(
      GuildId, RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void InviteToGuild(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &GuildId,
    const FString &TargetPlayerId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->InviteToGuild(
      GuildId,
      TargetPlayerId,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void ListGuildMembers(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &GuildId,
    ERedwoodGuildAndAllianceMemberState State,
    FRedwoodListGuildMembersOutputDynamicDelegate OnOutput
  ) {
    Subsystem->ListGuildMembers(
      GuildId,
      State,
      RW_FWD(
        FRedwoodListGuildMembersOutputDelegate,
        FRedwoodListGuildMembersOutput,
        OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void CreateGuild(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &GuildName,
    const FString &GuildTag,
    ERedwoodGuildInviteType InviteType,
    bool bListed,
    bool bMembershipPublic,
    FRedwoodCreateGuildOutputDynamicDelegate OnOutput
  ) {
    Subsystem->CreateGuild(
      GuildName,
      GuildTag,
      InviteType,
      bListed,
      bMembershipPublic,
      RW_FWD(
        FRedwoodCreateGuildOutputDelegate,
        FRedwoodCreateGuildOutput,
        OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void UpdateGuild(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &GuildId,
    const FString &GuildName,
    const FString &GuildTag,
    ERedwoodGuildInviteType InviteType,
    bool bListed,
    bool bMembershipPublic,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->UpdateGuild(
      GuildId,
      GuildName,
      GuildTag,
      InviteType,
      bListed,
      bMembershipPublic,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void KickPlayerFromGuild(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &GuildId,
    const FString &TargetPlayerId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->KickPlayerFromGuild(
      GuildId,
      TargetPlayerId,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void BanPlayerFromGuild(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &GuildId,
    const FString &TargetPlayerId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->BanPlayerFromGuild(
      GuildId,
      TargetPlayerId,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void UnbanPlayerFromGuild(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &GuildId,
    const FString &TargetPlayerId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->UnbanPlayerFromGuild(
      GuildId,
      TargetPlayerId,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void PromotePlayerToGuildAdmin(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &GuildId,
    const FString &TargetPlayerId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->PromotePlayerToGuildAdmin(
      GuildId,
      TargetPlayerId,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void DemotePlayerFromGuildAdmin(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &GuildId,
    const FString &TargetPlayerId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->DemotePlayerFromGuildAdmin(
      GuildId,
      TargetPlayerId,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  // ---------------------------------------------------------------------
  // Alliances (guilds of guilds)
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void ListAlliances(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &GuildIdFilter,
    FRedwoodListAlliancesOutputDynamicDelegate OnOutput
  ) {
    Subsystem->ListAlliances(
      GuildIdFilter,
      RW_FWD(
        FRedwoodListAlliancesOutputDelegate,
        FRedwoodListAlliancesOutput,
        OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void SearchForAlliances(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &SearchText,
    bool bIncludePartialMatches,
    FRedwoodListAlliancesOutputDynamicDelegate OnOutput
  ) {
    Subsystem->SearchForAlliances(
      SearchText,
      bIncludePartialMatches,
      RW_FWD(
        FRedwoodListAlliancesOutputDelegate,
        FRedwoodListAlliancesOutput,
        OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void CanAdminAlliance(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &AllianceId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->CanAdminAlliance(
      AllianceId, RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void CreateAlliance(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &AllianceName,
    const FString &GuildId,
    bool bInviteOnly,
    FRedwoodCreateAllianceOutputDynamicDelegate OnOutput
  ) {
    Subsystem->CreateAlliance(
      AllianceName,
      GuildId,
      bInviteOnly,
      RW_FWD(
        FRedwoodCreateAllianceOutputDelegate,
        FRedwoodCreateAllianceOutput,
        OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void UpdateAlliance(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &AllianceId,
    const FString &AllianceName,
    bool bInviteOnly,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->UpdateAlliance(
      AllianceId,
      AllianceName,
      bInviteOnly,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void ListAllianceGuilds(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &AllianceId,
    ERedwoodGuildAndAllianceMemberState State,
    FRedwoodListAllianceGuildsOutputDynamicDelegate OnOutput
  ) {
    Subsystem->ListAllianceGuilds(
      AllianceId,
      State,
      RW_FWD(
        FRedwoodListAllianceGuildsOutputDelegate,
        FRedwoodListAllianceGuildsOutput,
        OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void JoinAlliance(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &AllianceId,
    const FString &GuildId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->JoinAlliance(
      AllianceId,
      GuildId,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void LeaveAlliance(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &AllianceId,
    const FString &GuildId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->LeaveAlliance(
      AllianceId,
      GuildId,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void InviteGuildToAlliance(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &AllianceId,
    const FString &GuildId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->InviteGuildToAlliance(
      AllianceId,
      GuildId,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void KickGuildFromAlliance(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &AllianceId,
    const FString &GuildId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->KickGuildFromAlliance(
      AllianceId,
      GuildId,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void BanGuildFromAlliance(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &AllianceId,
    const FString &GuildId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->BanGuildFromAlliance(
      AllianceId,
      GuildId,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  UFUNCTION(ScriptCallable)
  static void UnbanGuildFromAlliance(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &AllianceId,
    const FString &GuildId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->UnbanGuildFromAlliance(
      AllianceId,
      GuildId,
      RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }

  // ---------------------------------------------------------------------
  // Proxies (the lobby path, as opposed to ticketing)
  // ---------------------------------------------------------------------

  UFUNCTION(ScriptCallable)
  static void ListProxies(
    URedwoodClientGameSubsystem *Subsystem,
    const TArray<FString> &PrivateProxyReferences,
    FRedwoodListProxiesOutputDynamicDelegate OnOutput
  ) {
    Subsystem->ListProxies(
      PrivateProxyReferences,
      RW_FWD(
        FRedwoodListProxiesOutputDelegate, FRedwoodListProxiesOutput, OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void CreateProxy(
    URedwoodClientGameSubsystem *Subsystem,
    bool bJoinSession,
    const FRedwoodCreateProxyInput &Parameters,
    FRedwoodCreateProxyOutputDynamicDelegate OnOutput
  ) {
    Subsystem->CreateProxy(
      bJoinSession,
      Parameters,
      RW_FWD(
        FRedwoodCreateProxyOutputDelegate,
        FRedwoodCreateProxyOutput,
        OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void JoinProxyWithSingleInstance(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &ProxyReference,
    const FString &Password,
    FRedwoodJoinServerOutputDynamicDelegate OnOutput
  ) {
    Subsystem->JoinProxyWithSingleInstance(
      ProxyReference,
      Password,
      RW_FWD(
        FRedwoodJoinServerOutputDelegate, FRedwoodJoinServerOutput, OnOutput
      )
    );
  }

  UFUNCTION(ScriptCallable)
  static void StopProxy(
    URedwoodClientGameSubsystem *Subsystem,
    const FString &ServerProxyId,
    FRedwoodErrorOutputDynamicDelegate OnOutput
  ) {
    Subsystem->StopProxy(
      ServerProxyId, RW_FWD(FRedwoodErrorOutputDelegate, FString, OnOutput)
    );
  }
};

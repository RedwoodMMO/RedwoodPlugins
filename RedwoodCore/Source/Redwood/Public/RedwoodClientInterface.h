// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodModule.h"
#include "Types/RedwoodTypes.h"

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "TimerManager.h"

#include "SocketIOFunctionLibrary.h"
#include "SocketIONative.h"

#include "RedwoodClientInterface.generated.h"

// This class is separated from URedwoodClientGameSubsystem
// so that we can use it in automated tests without instantiating
// a whole world.

UCLASS()
class REDWOOD_API URedwoodClientInterface : public UObject,
                                            public FTickableGameObject {
  GENERATED_BODY()

public:
  void Deinitialize();

  // FTickableGameObject begin
  virtual void Tick(float DeltaTime) override;
  virtual TStatId GetStatId() const override;
  virtual bool IsTickableInEditor() const override {
    return true;
  }
  // FTickableGameObject end

  void InitializeDirectorConnection(
    FRedwoodSocketConnectedDelegate OnDirectorConnected
  );

  bool IsDirectorConnected();

  TSharedPtr<FSocketIONative> GetDirectorConnection() const {
    return Director;
  }

  TSharedPtr<FSocketIONative> GetRealmConnection() const {
    return Realm;
  }

  FRedwoodDynamicDelegate OnPingsReceived;

  FRedwoodConnectToServerDynamicDelegate OnRequestToJoinServer;
  FRedwoodStitchToServerDynamicDelegate OnRequestToStitchServer;

  FRedwoodDynamicDelegate OnDirectorConnectionLost;
  FRedwoodDynamicDelegate OnDirectorConnectionReestablished;
  FRedwoodDynamicDelegate OnRealmConnectionLost;
  FRedwoodDynamicDelegate OnRealmConnectionReestablished;

  void Register(
    const FString &Username,
    const FString &Password,
    FRedwoodAuthUpdateDelegate OnUpdate
  );

  void AttemptAutoLogin(FRedwoodAuthUpdateDelegate OnUpdate);

  void Login(
    const FString &Username,
    const FString &PasswordOrToken,
    const FString &Provider,
    bool bRememberMe,
    FRedwoodAuthUpdateDelegate OnUpdate,
    bool bBypassProviderCheck = false
  );

  void LoginWithDiscord(bool bRememberMe, FRedwoodAuthUpdateDelegate OnUpdate);
  void LoginWithTwitch(bool bRememberMe, FRedwoodAuthUpdateDelegate OnUpdate);

  FString GetNickname();

  void Logout();

  bool IsLoggedIn();

  FString GetPlayerId();
  FString GetCharacterId();
  FString GetCharacterName();
  FString GetRealmId();

  void CancelWaitingForAccountVerification();

  void SearchForPlayers(
    FString UsernameOrNickname,
    bool bIncludePartialMatches,
    FRedwoodListPlayersOutputDelegate OnOutput
  );
  void SearchForPlayerById(
    FString TargetPlayerId, FRedwoodPlayerOutputDelegate OnOutput
  );

  void ListFriends(
    ERedwoodFriendListType Filter, FRedwoodListPlayersOutputDelegate OnOutput
  );

  void RequestFriend(
    FString OtherPlayerId, FRedwoodErrorOutputDelegate OnOutput
  );

  void RemoveFriend(
    FString OtherPlayerId, FRedwoodErrorOutputDelegate OnOutput
  );

  void RespondToFriendRequest(
    FString OtherPlayerId, bool bAccept, FRedwoodErrorOutputDelegate OnOutput
  );

  void SetPlayerBlocked(
    FString OtherPlayerId, bool bBlocked, FRedwoodErrorOutputDelegate OnOutput
  );

  void ListRealmContacts(FRedwoodListRealmContactsOutputDelegate OnOutput);

  void AddRealmContact(
    FString OtherCharacterId,
    bool bBlocked,
    FRedwoodErrorOutputDelegate OnOutput
  );

  void RemoveRealmContact(
    FString OtherCharacterId, FRedwoodErrorOutputDelegate OnOutput
  );

  void ListGuilds(
    bool bOnlyPlayersGuilds, FRedwoodListGuildsOutputDelegate OnOutput
  );

  void SearchForGuilds(
    FString SearchText,
    bool bIncludePartialMatches,
    FRedwoodListGuildsOutputDelegate OnOutput
  );

  void GetGuild(FString GuildId, FRedwoodGetGuildOutputDelegate OnOutput);

  void SetSelectedGuild(FString GuildId, FRedwoodErrorOutputDelegate OnOutput);

  void GetSelectedGuild(FRedwoodGetGuildOutputDelegate OnOutput);

  void JoinGuild(FString GuildId, FRedwoodErrorOutputDelegate OnOutput);

  void InviteToGuild(
    FString GuildId,
    FString TargetPlayerId,
    FRedwoodErrorOutputDelegate OnOutput
  );

  void LeaveGuild(FString GuildId, FRedwoodErrorOutputDelegate OnOutput);

  void ListGuildMembers(
    FString GuildId,
    ERedwoodGuildAndAllianceMemberState State,
    FRedwoodListGuildMembersOutputDelegate OnOutput
  );

  void CreateGuild(
    FString GuildName,
    FString GuildTag,
    ERedwoodGuildInviteType InviteType,
    bool bListed,
    bool bMembershipPublic,
    FRedwoodCreateGuildOutputDelegate OnOutput
  );

  void UpdateGuild(
    FString GuildId,
    FString GuildName,
    FString GuildTag,
    ERedwoodGuildInviteType InviteType,
    bool bListed,
    bool bMembershipPublic,
    FRedwoodErrorOutputDelegate OnOutput
  );

  void KickPlayerFromGuild(
    FString GuildId, FString TargetId, FRedwoodErrorOutputDelegate OnOutput
  );

  void BanPlayerFromGuild(
    FString GuildId, FString TargetId, FRedwoodErrorOutputDelegate OnOutput
  );

  void UnbanPlayerFromGuild(
    FString GuildId, FString TargetId, FRedwoodErrorOutputDelegate OnOutput
  );

  void PromotePlayerToGuildAdmin(
    FString GuildId, FString TargetId, FRedwoodErrorOutputDelegate OnOutput
  );

  void DemotePlayerFromGuildAdmin(
    FString GuildId, FString TargetId, FRedwoodErrorOutputDelegate OnOutput
  );

  void ListAlliances(
    FString GuildIdFilter, FRedwoodListAlliancesOutputDelegate OnOutput
  );

  void SearchForAlliances(
    FString SearchText,
    bool bIncludePartialMatches,
    FRedwoodListAlliancesOutputDelegate OnOutput
  );

  void CanAdminAlliance(
    FString AllianceId, FRedwoodErrorOutputDelegate OnOutput
  );

  void CreateAlliance(
    FString AllianceName,
    FString GuildId,
    bool bInviteOnly,
    FRedwoodCreateAllianceOutputDelegate OnOutput
  );

  void UpdateAlliance(
    FString AllianceId,
    FString AllianceName,
    bool bInviteOnly,
    FRedwoodErrorOutputDelegate OnOutput
  );

  void KickGuildFromAlliance(
    FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
  );

  void BanGuildFromAlliance(
    FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
  );

  void UnbanGuildFromAlliance(
    FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
  );

  void ListAllianceGuilds(
    FString AllianceId,
    ERedwoodGuildAndAllianceMemberState State,
    FRedwoodListAllianceGuildsOutputDelegate OnOutput
  );

  void JoinAlliance(
    FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
  );

  void LeaveAlliance(
    FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
  );

  void InviteGuildToAlliance(
    FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
  );

  void ListRealms(FRedwoodListRealmsOutputDelegate OnOutput);

  void InitializeConnectionForFirstRealm(
    FRedwoodSocketConnectedDelegate OnRealmConnected
  );
  void InitializeRealmConnection(
    FRedwoodRealm InRealm, FRedwoodSocketConnectedDelegate OnRealmConnected
  );

  bool IsRealmConnected(FRedwoodRealm &OutRealm);
  bool IsRealmConnected();

  TMap<FString, float> GetRegions();

  void ListCharacters(FRedwoodListCharactersOutputDelegate OnOutput);

  void ListArchivedCharacters(FRedwoodListCharactersOutputDelegate OnOutput);

  void CreateCharacter(
    FString Name,
    USIOJsonObject *CharacterCreatorData,
    FRedwoodGetCharacterOutputDelegate OnOutput
  );

  void SetCharacterArchived(
    FString CharacterId, bool bArchived, FRedwoodErrorOutputDelegate OnOutput
  );

  void GetCharacterData(
    FString CharacterIdOrName, FRedwoodGetCharacterOutputDelegate OnOutput
  );

  void SetCharacterData(
    FString CharacterId,
    FString Name,
    USIOJsonObject *CharacterCreatorData,
    FRedwoodGetCharacterOutputDelegate OnOutput
  );

  void SetSelectedCharacter(FString CharacterId);

  void JoinMatchmaking(
    FString ProfileId,
    TArray<FString> InRegions,
    FRedwoodTicketingUpdateDelegate OnUpdate
  );

  void JoinQueue(
    FString ProxyId,
    FString ZoneName,
    bool bTransferWholeParty,
    FRedwoodTicketingUpdateDelegate OnUpdate
  );

  void JoinCustom(
    bool bTransferWholeParty,
    TArray<FString> InRegions,
    FRedwoodTicketingUpdateDelegate OnUpdate
  );

  void LeaveTicketing(FRedwoodErrorOutputDelegate OnOutput);

  void ListProxies(
    TArray<FString> PrivateProxyReferences,
    FRedwoodListProxiesOutputDelegate OnOutput
  );
  void CreateProxy(
    bool bJoinSession,
    FRedwoodCreateProxyInput Parameters,
    FRedwoodCreateProxyOutputDelegate OnOutput
  );
  void JoinProxyWithSingleInstance(
    FString ProxyReference,
    FString Password,
    FRedwoodJoinServerOutputDelegate OnOutput
  );
  void StopProxy(FString ServerProxyId, FRedwoodErrorOutputDelegate OnOutput);

  FString GetConnectionConsoleCommand();
  FURL GetConnectionURL();

  FRedwoodPartyInvitedDynamicDelegate OnPartyInvited;
  FRedwoodPartyUpdatedDynamicDelegate OnPartyUpdated;
  FRedwoodDynamicDelegate OnPartyKicked;
  FRedwoodPartyEmoteReceivedDynamicDelegate OnPartyEmoteReceived;
  FRedwoodParty GetCachedParty() {
    return CurrentParty;
  }
  void GetOrCreateParty(
    bool bCreateIfNotInParty, FRedwoodGetPartyOutputDelegate OnOutput
  );
  void LeaveParty(FRedwoodErrorOutputDelegate OnOutput);
  void InviteToParty(
    FString TargetPlayerId, FRedwoodErrorOutputDelegate OnOutput
  );
  void ListPartyInvites(FRedwoodListPartyInvitesOutputDelegate OnOutput);
  void RespondToPartyInvite(
    FString PartyId, bool bAccept, FRedwoodGetPartyOutputDelegate OnOutput
  );
  void PromoteToPartyLeader(
    FString TargetPlayerId, FRedwoodErrorOutputDelegate OnOutput
  );
  void KickFromParty(
    FString TargetPlayerId, FRedwoodErrorOutputDelegate OnOutput
  );
  void SetPartyData(
    FString LootType,
    USIOJsonObject *PartyData,
    FRedwoodGetPartyOutputDelegate OnOutput
  );
  void SendEmoteToParty(FString Emote);

private:
  bool bSentDirectorConnected;
  bool bDirectorDisconnected = true;
  bool bSentInitialDirectorConnectionFailureLog = false;
  bool bSentRealmConnected;
  bool bRealmDisconnected = true;
  bool bSentInitialRealmConnectionFailureLog = false;
  TSharedPtr<FSocketIONative> Director;
  TSharedPtr<FSocketIONative> Realm;

  void InitiateRealmHandshake(
    FRedwoodRealm InRealm, FRedwoodSocketConnectedDelegate OnRealmConnected
  );
  void BindRealmEvents();
  void FinalizeRealmHandshake(
    FString Token, FRedwoodSocketConnectedDelegate OnRealmConnected
  );

  UFUNCTION()
  void BeginRealmReauthentication();
  FTimerHandle ReauthenticationAttemptTimer;

  void HandleRegionsChanged(
    const FString &Event, const TSharedPtr<FJsonValue> &Message
  );

  void InitiatePings();
  UFUNCTION()
  void HandlePingResult(FString TargetAddress, float RTT);

  TMap<FString, TSharedPtr<FRedwoodRegionLatency>> Regions;
  TMap<FString, float> PingAverages;
  FTimerHandle PingTimer;
  int PingAttemptsLeft;

  void AttemptJoinMatchmaking();
  void AttemptJoinCustom();
  FRedwoodTicketingUpdateDelegate OnTicketingUpdate;
  FString TicketingProfileId;
  TArray<FString> TicketingRegions;
  bool bTicketingTransferWholeParty = false;

  FString ServerConnection;
  FString ServerToken;

  FRedwoodAuthUpdateDelegate OnAccountVerified;
  bool bAuthenticated = false;
  FString PlayerId;
  FString AuthToken;
  FString SelectedCharacterId;
  FString Nickname;
  FString CurrentRealmId;
  FRedwoodRealm CurrentRealm;

  TMap<FString, FString> CharacterNamesById;

  FTimerManager TimerManager;

  UPROPERTY()
  FRedwoodParty CurrentParty;
};

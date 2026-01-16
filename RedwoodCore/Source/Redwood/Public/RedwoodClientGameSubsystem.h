// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodModule.h"
#include "Types/RedwoodTypes.h"

#include "CoreMinimal.h"
#include "Engine/WorldInitializationValues.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "RedwoodClientGameSubsystem.generated.h"

class URedwoodClientInterface;

UCLASS(BlueprintType)
class REDWOOD_API URedwoodClientGameSubsystem : public UGameInstanceSubsystem {
  GENERATED_BODY()

public:
  // Begin USubsystem
  virtual void Initialize(FSubsystemCollectionBase &Collection) override;
  virtual void Deinitialize() override;
  // End USubsystem

  void InitializeDirectorConnection(
    FRedwoodSocketConnectedDelegate OnDirectorConnected
  );

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsDirectorConnected();

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodDynamicDelegate OnPingsReceived;

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodConnectToServerDynamicDelegate OnRequestToJoinServer;

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodDynamicDelegate OnDirectorConnectionLost;

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodDynamicDelegate OnDirectorConnectionReestablished;

  // There is no realm established delegate; you'll have to reinitialize
  // the realm connection from the director to restart the handshake.
  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodDynamicDelegate OnRealmConnectionLost;

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodPartyInvitedDynamicDelegate OnPartyInvited;

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodPartyUpdatedDynamicDelegate OnPartyUpdated;

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodDynamicDelegate OnPartyKicked;

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodPartyEmoteReceivedDynamicDelegate OnPartyEmoteReceived;

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
    FRedwoodAuthUpdateDelegate OnUpdate
  );

  void LoginWithDiscord(bool bRememberMe, FRedwoodAuthUpdateDelegate OnUpdate);
  void LoginWithTwitch(bool bRememberMe, FRedwoodAuthUpdateDelegate OnUpdate);

  UFUNCTION(BlueprintPure, Category = "Redwood")
  FString GetNickname();

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void Logout();

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsLoggedIn();

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void CancelWaitingForAccountVerification();

  UFUNCTION(BlueprintPure, Category = "Redwood")
  FString GetPlayerId();

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

  void GetSelectedGuild(FRedwoodGetGuildOutputDelegate OnOutput);

  void SetSelectedGuild(FString GuildId, FRedwoodErrorOutputDelegate OnOutput);

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
    FString GuildId,
    FString TargetPlayerId,
    FRedwoodErrorOutputDelegate OnOutput
  );

  void BanPlayerFromGuild(
    FString GuildId,
    FString TargetPlayerId,
    FRedwoodErrorOutputDelegate OnOutput
  );

  void UnbanPlayerFromGuild(
    FString GuildId,
    FString TargetPlayerId,
    FRedwoodErrorOutputDelegate OnOutput
  );

  void PromotePlayerToGuildAdmin(
    FString GuildId,
    FString TargetPlayerId,
    FRedwoodErrorOutputDelegate OnOutput
  );

  void DemotePlayerFromGuildAdmin(
    FString GuildId,
    FString TargetPlayerId,
    FRedwoodErrorOutputDelegate OnOutput
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

  // This is a helper function for scenarios that only have a single realm
  // (i.e. games that only have one realm or during dev/testing environments).
  // It calls ListRealms and then InitializeRealmConnection for the first realm returned.
  void InitializeConnectionForFirstRealm(
    FRedwoodSocketConnectedDelegate OnRealmConnected
  );
  void InitializeRealmConnection(
    FRedwoodRealm InRealm, FRedwoodSocketConnectedDelegate OnRealmConnected
  );

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsRealmConnected(FRedwoodRealm &OutRealm);
  bool IsRealmConnected();

  UFUNCTION(BlueprintCallable, Category = "Redwood")
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

  UFUNCTION(BlueprintCallable, Category = "Redwood")
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

  UFUNCTION(BlueprintPure, Category = "Redwood")
  FRedwoodParty GetCachedParty();

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
  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void SendEmoteToParty(FString Emote);

  void GetDirectorGlobalData(FRedwoodGetGlobalDataOutputDelegate OnOutput);
  void GetRealmGlobalData(FRedwoodGetGlobalDataOutputDelegate OnOutput);

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  FString GetConnectionConsoleCommand();

  URedwoodClientInterface *GetClientInterface() {
    return ClientInterface;
  }

private:
  UPROPERTY()
  URedwoodClientInterface *ClientInterface;

  UFUNCTION()
  void HandlePingsReceived();

  UFUNCTION()
  void HandleOnDirectorConnectionLost();

  UFUNCTION()
  void HandleOnDirectorConnectionReestablished();

  UFUNCTION()
  void HandleOnRealmConnectionLost();

  UFUNCTION()
  void HandleRequestToJoinServer(FString ConsoleCommand);

  UFUNCTION()
  void HandleOnPartyInvited(FRedwoodPartyInvite Invite);

  UFUNCTION()
  void HandleOnPartyUpdated(FRedwoodParty Party);

  UFUNCTION()
  void HandleOnPartyKicked();

  UFUNCTION()
  void HandleOnPartyEmoteReceived(
    const FString &PlayerId, const FString &Emote
  );

  void HandleOnWorldAdded(UWorld *World, FWorldInitializationValues IVS);
  void HandleOnWorldBeginPlay(bool bBegunPlay);
};

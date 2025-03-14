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

  UFUNCTION(BlueprintPure, Category = "Redwood")
  FString GetNickname();

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void Logout();

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsLoggedIn();

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void CancelWaitingForAccountVerification();

  void SearchForPlayers(
    FString UsernameOrNickname,
    bool bIncludePartialMatches,
    FRedwoodListFriendsOutputDelegate OnOutput
  );

  void ListFriends(
    ERedwoodFriendListType Filter, FRedwoodListFriendsOutputDelegate OnOutput
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
    FString CharacterId, FRedwoodGetCharacterOutputDelegate OnOutput
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
    FString ProxyId, FString ZoneName, FRedwoodTicketingUpdateDelegate OnUpdate
  );

  void LeaveTicketing(FRedwoodErrorOutputDelegate OnOutput);

  void ListServers(
    TArray<FString> PrivateServerReferences,
    FRedwoodListServersOutputDelegate OnOutput
  );
  void CreateServer(
    bool bJoinSession,
    FRedwoodCreateServerInput Parameters,
    FRedwoodCreateServerOutputDelegate OnOutput
  );
  void JoinServerInstance(
    FString ServerReference,
    FString Password,
    FRedwoodJoinServerOutputDelegate OnOutput
  );
  void StopServer(FString ServerProxyId, FRedwoodErrorOutputDelegate OnOutput);

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

  void HandleOnWorldAdded(UWorld *World, FWorldInitializationValues IVS);
  void HandleOnWorldBeginPlay(bool bBegunPlay);

  UFUNCTION()
  void ReportOnlineStatus();
};

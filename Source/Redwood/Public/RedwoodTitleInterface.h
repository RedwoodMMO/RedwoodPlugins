// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodModule.h"
#include "Types/RedwoodTypes.h"

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"

#include "SocketIOFunctionLibrary.h"
#include "SocketIONative.h"

#include "RedwoodTitleInterface.generated.h"

// This class is separated from URedwoodTitleGameSubsystem
// so that we can use it in automated tests without instantiating
// a whole world.

class FTimerManager;

UCLASS()
class REDWOOD_API URedwoodTitleInterface : public UObject,
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

  FRedwoodDynamicDelegate OnPingsReceived;

  FRedwoodConnectToServerDynamicDelegate OnRequestToJoinServer;

  void Register(
    const FString &Username,
    const FString &Password,
    FRedwoodAuthUpdateDelegate OnUpdate
  );

  void AttemptAutoLogin(FRedwoodAuthUpdateDelegate OnUpdate);

  void Login(
    const FString &Username,
    const FString &PasswordOrToken,
    bool bRememberMe,
    FRedwoodAuthUpdateDelegate OnUpdate
  );

  void Logout();

  bool IsLoggedIn();

  FString GetPlayerId();

  void CancelWaitingForAccountVerification();

  void ListRealms(FRedwoodListRealmsOutputDelegate OnOutput);

  void InitializeConnectionForFirstRealm(
    FRedwoodSocketConnectedDelegate OnRealmConnected
  );
  void InitializeRealmConnection(
    FRedwoodRealm InRealm, FRedwoodSocketConnectedDelegate OnRealmConnected
  );

  bool IsRealmConnected();

  TMap<FString, float> GetRegions();

  static FRedwoodCharacter ParseCharacter(TSharedPtr<FJsonObject> CharacterObj);

  void ListCharacters(FRedwoodListCharactersOutputDelegate OnOutput);

  void CreateCharacter(
    USIOJsonObject *Metadata,
    USIOJsonObject *EquippedInventory,
    USIOJsonObject *NonequippedInventory,
    USIOJsonObject *Data,
    FRedwoodGetCharacterOutputDelegate OnOutput
  );

  void GetCharacterData(
    FString CharacterId, FRedwoodGetCharacterOutputDelegate OnOutput
  );

  void SetCharacterData(
    FString CharacterId,
    USIOJsonObject *Metadata,
    USIOJsonObject *EquippedInventory,
    USIOJsonObject *NonequippedInventory,
    USIOJsonObject *Data,
    FRedwoodGetCharacterOutputDelegate OnOutput
  );

  void SetSelectedCharacter(FString CharacterId);

  void JoinMatchmaking(
    TArray<FString> ProfileTypes,
    TArray<FString> InRegions,
    FRedwoodTicketingUpdateDelegate OnUpdate
  );

  void LeaveTicketing(FRedwoodErrorOutputDelegate OnOutput);

  static FRedwoodGameServerProxy ParseServerProxy(
    TSharedPtr<FJsonObject> ServerProxy
  );
  static FRedwoodGameServerInstance ParseServerInstance(
    TSharedPtr<FJsonObject> ServerInstance
  );
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

  FString GetConnectionConsoleCommand();

private:
  TSharedPtr<FSocketIONative> Director;
  TSharedPtr<FSocketIONative> Realm;

  void InitiateRealmHandshake(
    FRedwoodRealm InRealm, FRedwoodSocketConnectedDelegate OnRealmConnected
  );
  void BindRealmEvents();
  void FinalizeRealmHandshake(
    FString Token, FRedwoodSocketConnectedDelegate OnRealmConnected
  );

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
  FRedwoodTicketingUpdateDelegate OnTicketingUpdate;
  TArray<FString> TicketingProfileTypes;
  TArray<FString> TicketingRegions;

  FString ServerConnection;
  FString ServerToken;

  FRedwoodAuthUpdateDelegate OnAccountVerified;
  FString PlayerId;
  FString AuthToken;
  FString SelectedCharacterId;

  FTimerManager TimerManager;
};

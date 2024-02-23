// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodModule.h"
#include "RedwoodTypes.h"

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SocketIOFunctionLibrary.h"
#include "SocketIONative.h"

#include "RedwoodTitleGameSubsystem.generated.h"

UCLASS()
class URedwoodTitleGameSubsystem : public UGameInstanceSubsystem {
  GENERATED_BODY()

public:
  typedef TDelegate<void(const FRedwoodAuthUpdate &)> FRedwoodOnAuthUpdate;
  typedef TDelegate<void(const FRedwoodCharactersResult &)>
    FRedwoodOnListCharacters;
  typedef TDelegate<void(const FRedwoodCharacterResult &)>
    FRedwoodOnGetCharacter;
  typedef TDelegate<void(const FRedwoodTicketingUpdate &)>
    FRedwoodOnTicketingUpdate;
  typedef TDelegate<void(const FRedwoodRealmsResult &)> FRedwoodOnListRealms;
  typedef TDelegate<void(const FRedwoodSocketConnected &)>
    FRedwoodOnSocketConnected;
  typedef TDelegate<void(const FRedwoodListServers &)> FRedwoodOnListServers;
  typedef TDelegate<void(const FRedwoodGetServer &)> FRedwoodOnGetServer;
  typedef TDelegate<void(const FString &)> FRedwoodOnSimpleResult;

  UDELEGATE()
  DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FRedwoodPingResult, FString, Region, float, RTT
  );

  // Begin USubsystem
  virtual void Initialize(FSubsystemCollectionBase &Collection) override;
  virtual void Deinitialize() override;
  // End USubsystem

  void InitializeDirectorConnection(
    FRedwoodOnSocketConnected OnDirectorConnected
  );

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodPingResult OnPingResultReceived;

  void Register(
    const FString &Username,
    const FString &Password,
    FRedwoodOnAuthUpdate OnUpdate
  );

  void Login(
    const FString &Username,
    const FString &PasswordOrToken,
    FRedwoodOnAuthUpdate OnUpdate
  );

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void CancelWaitingForAccountVerification();

  void ListRealms(FRedwoodOnListRealms OnResult);

  void InitializeSingleRealmConnection(
    FRedwoodOnSocketConnected OnRealmConnected
  );
  void InitializeRealmConnection(
    FRedwoodRealm InRealm, FRedwoodOnSocketConnected OnRealmConnected
  );

  void ListCharacters(FRedwoodOnListCharacters OnResult);

  void CreateCharacter(USIOJsonObject *Data, FRedwoodOnGetCharacter OnResult);

  void GetCharacterData(FString CharacterId, FRedwoodOnGetCharacter OnResult);

  void SetCharacterData(
    FString CharacterId, USIOJsonObject *Data, FRedwoodOnGetCharacter OnResult
  );

  void JoinTicketing(FString Profile, FRedwoodOnTicketingUpdate OnUpdate);

  static FRedwoodGameServerProxy ParseServerProxy(
    TSharedPtr<FJsonObject> ServerProxy
  );
  static FRedwoodGameServerInstance ParseServerInstance(
    TSharedPtr<FJsonObject> ServerInstance
  );
  void ListServers(
    TArray<FString> PrivateServerReferences, FRedwoodOnListServers OnResult
  );
  void CreateServer(
    FRedwoodCreateServerParameters Parameters, FRedwoodOnGetServer OnResult
  );
  void GetServerInstance(
    FString ServerReference, FString Password, FRedwoodOnGetServer OnResult
  );
  void StopServer(FString ServerProxyId, FRedwoodOnSimpleResult OnResult);

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  FString GetConnectionString(FString CharacterId);

private:
  TSharedPtr<FSocketIONative> Director;
  TSharedPtr<FSocketIONative> Realm;

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

  void AttemptJoinTicketing();
  FRedwoodOnTicketingUpdate OnTicketingUpdate;
  FString TicketingProfile;
  FString TicketingConnection;
  FString TicketingToken;

  FRedwoodOnAuthUpdate OnAccountVerified;
  FString PlayerId;
  FString AuthToken;
};

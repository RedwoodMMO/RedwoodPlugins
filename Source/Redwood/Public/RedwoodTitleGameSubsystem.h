// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodModule.h"
#include "Types/RedwoodTypes.h"

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SocketIOFunctionLibrary.h"
#include "SocketIONative.h"

#include "RedwoodTitleGameSubsystem.generated.h"

UCLASS()
class URedwoodTitleGameSubsystem : public UGameInstanceSubsystem {
  GENERATED_BODY()

public:
  UDELEGATE()
  DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FRedwoodPingResult, FString, Region, float, RTT
  );

  // Begin USubsystem
  virtual void Initialize(FSubsystemCollectionBase &Collection) override;
  virtual void Deinitialize() override;
  // End USubsystem

  void InitializeDirectorConnection(
    FRedwoodSocketConnectedDelegate OnDirectorConnected
  );

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodPingResult OnPingResultReceived;

  void Register(
    const FString &Username,
    const FString &Password,
    FRedwoodAuthUpdateDelegate OnUpdate
  );

  void Login(
    const FString &Username,
    const FString &PasswordOrToken,
    FRedwoodAuthUpdateDelegate OnUpdate
  );

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void CancelWaitingForAccountVerification();

  void ListRealms(FRedwoodListRealmsOutputDelegate OnOutput);

  void InitializeSingleRealmConnection(
    FRedwoodSocketConnectedDelegate OnRealmConnected
  );
  void InitializeRealmConnection(
    FRedwoodRealm InRealm, FRedwoodSocketConnectedDelegate OnRealmConnected
  );

  void ListCharacters(FRedwoodListCharactersOutputDelegate OnOutput);

  void CreateCharacter(
    USIOJsonObject *Data, FRedwoodGetCharacterOutputDelegate OnOutput
  );

  void GetCharacterData(
    FString CharacterId, FRedwoodGetCharacterOutputDelegate OnOutput
  );

  void SetCharacterData(
    FString CharacterId,
    USIOJsonObject *Data,
    FRedwoodGetCharacterOutputDelegate OnOutput
  );

  void JoinTicketing(FString Profile, FRedwoodTicketingUpdateDelegate OnUpdate);

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
    FRedwoodCreateServerInput Parameters,
    FRedwoodGetServerOutputDelegate OnOutput
  );
  void GetServerInstance(
    FString ServerReference,
    FString Password,
    FRedwoodGetServerOutputDelegate OnOutput
  );
  void StopServer(FString ServerProxyId, FRedwoodErrorOutputDelegate OnOutput);

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
  FRedwoodTicketingUpdateDelegate OnTicketingUpdate;
  FString TicketingProfile;
  FString TicketingConnection;
  FString TicketingToken;

  FRedwoodAuthUpdateDelegate OnAccountVerified;
  FString PlayerId;
  FString AuthToken;
};

// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodModule.h"
#include "Types/RedwoodTypes.h"

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SocketIOFunctionLibrary.h"
#include "SocketIONative.h"

#include "RedwoodTitleGameSubsystem.generated.h"

UCLASS(BlueprintType)
class URedwoodTitleGameSubsystem : public UGameInstanceSubsystem {
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
  FRedwoodDynamicDelegate OnRequestToJoinServer;

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
  void Logout();

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsLoggedIn();

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void CancelWaitingForAccountVerification();

  void ListRealms(FRedwoodListRealmsOutputDelegate OnOutput);

  void InitializeSingleRealmConnection(
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

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void SetSelectedCharacter(FString CharacterId);

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
    bool bJoinSession,
    FRedwoodCreateServerInput Parameters,
    FRedwoodCreateServerOutputDelegate OnOutput
  );
  void GetServerInstance(
    FString ServerReference,
    FString Password,
    bool bJoinSession,
    FRedwoodGetServerOutputDelegate OnOutput
  );
  void StopServer(FString ServerProxyId, FRedwoodErrorOutputDelegate OnOutput);

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  FString GetConnectionConsoleCommand();

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
  FString ServerConnection;
  FString ServerToken;

  FRedwoodAuthUpdateDelegate OnAccountVerified;
  FString PlayerId;
  FString AuthToken;
  FString SelectedCharacterId;
};

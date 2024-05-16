// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodModule.h"
#include "Types/RedwoodTypes.h"

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "RedwoodTitleGameSubsystem.generated.h"

class URedwoodTitleInterface;

UCLASS(BlueprintType)
class REDWOOD_API URedwoodTitleGameSubsystem : public UGameInstanceSubsystem {
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

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void Logout();

  UFUNCTION(BlueprintPure, Category = "Redwood")
  bool IsLoggedIn();

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void CancelWaitingForAccountVerification();

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

  void JoinMatchmaking(
    FString ProfileId,
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

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  FString GetConnectionConsoleCommand();

private:
  UPROPERTY()
  URedwoodTitleInterface *TitleInterface;

  UFUNCTION()
  void HandlePingsReceived();

  UFUNCTION()
  void HandleRequestToJoinServer(FString ConsoleCommand);
};

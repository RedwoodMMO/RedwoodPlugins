// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

#include "SIOJsonObject.h"
#include "SocketIOClientComponent.h"

#include "RedwoodTypes.h"

#include "RedwoodTitlePlayerController.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodSimpleResponse, FString, Error
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
  FRedwoodAuthUpdate, ERedwoodAuthUpdateType, Type, FString, Message
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
  FPingResultSignature, FString, Region, float, RTT
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
  FRedwoodCharactersResponse,
  FString,
  Error,
  TArray<FRedwoodPlayerCharacter>,
  Characters
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
  FRedwoodCharacterResponse, FString, Error, FRedwoodPlayerCharacter, Character
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
  FRedwoodLobbyUpdate, ERedwoodLobbyUpdateType, Type, FString, Message
);

UCLASS()
class REDWOOD_API ARedwoodTitlePlayerController : public APlayerController {
  GENERATED_UCLASS_BODY()

public:
  //~ Begin AActor Interface
  virtual void BeginPlay() override;
  //~ End AActor Interface

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  FString DirectorAddress = "ws://localhost:3000";

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FSIOCOpenEventSignature OnDirectorConnected;

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FPingResultSignature OnPingResultReceived;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  int PingAttempts = 1;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  float PingTimeoutSec = 3;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  float PingFrequencySec = 10;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  bool bUseWebsocketRegionPings = true;

  void Register(
    const FString &Username,
    const FString &Password,
    FRedwoodAuthUpdate OnUpdated
  );

  void Login(
    const FString &Username,
    const FString &PasswordOrToken,
    FRedwoodAuthUpdate OnUpdated
  );

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void CancelWaitingForAccountVerification();

  void ListCharacters(FRedwoodCharactersResponse OnResponse);

  void CreateCharacter(
    TSharedPtr<FJsonObject> Data, FRedwoodCharacterResponse OnResponse
  );

  void GetCharacterData(
    FString CharacterId, FRedwoodCharacterResponse OnResponse
  );

  void SetCharacterData(
    FString CharacterId,
    TSharedPtr<FJsonObject> Data,
    FRedwoodCharacterResponse OnResponse
  );

  void JoinLobby(FRedwoodLobbyUpdate OnUpdate);

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  FString GetMatchConnectionString(FString CharacterId);

private:
  void HandleDirectorConnected(
    FString SocketId, FString SessionId, bool bIsReconnection
  );

  void HandleRegionsChanged(
    const FString &Event, const TSharedPtr<FJsonValue> &Message
  );

  void InitiatePings();
  void HandlePingResult(FString TargetAddress, float RTT);

  UPROPERTY()
  USocketIOClientComponent *DirectorSocketIOComponent;

  TMap<FString, FDataCenterLatency> DataCenters;
  TMap<FString, float> PingAverages;
  FTimerHandle PingTimer;
  int PingAttemptsLeft;

  void AttemptJoinLobby();
  FRedwoodLobbyUpdate OnLobbyUpdate;
  FString LobbyConnection;

  FString LobbyToken;

  FRedwoodAuthUpdate OnAccountVerified;
  FString PlayerId;
  FString AuthToken;
};
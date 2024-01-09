// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

#include "SIOJsonObject.h"
#include "SocketIOClientComponent.h"

#include "RedwoodTypes.h"

#include "RedwoodTitlePlayerController.generated.h"

UDELEGATE(BlueprintType)
DECLARE_DYNAMIC_DELEGATE_OneParam(FRedwoodSimpleResponse, FString, Error);

UDELEGATE(BlueprintType)
DECLARE_DYNAMIC_DELEGATE_TwoParams(
  FRedwoodAuthUpdate, ERedwoodAuthUpdateType, Type, FString, Message
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
  FPingResultSignature, FString, Region, float, RTT
);

UDELEGATE(BlueprintType)
DECLARE_DYNAMIC_DELEGATE_TwoParams(
  FRedwoodCharactersResponse,
  FString,
  Error,
  const TArray<FRedwoodPlayerCharacter> &,
  Characters
);

UDELEGATE(BlueprintType)
DECLARE_DYNAMIC_DELEGATE_TwoParams(
  FRedwoodCharacterResponse, FString, Error, FRedwoodPlayerCharacter, Character
);

UDELEGATE(BlueprintType)
DECLARE_DYNAMIC_DELEGATE_TwoParams(
  FRedwoodLobbyUpdate, ERedwoodLobbyUpdateType, Type, FString, Message
);

UCLASS()
class REDWOOD_API ARedwoodTitlePlayerController : public APlayerController {
  GENERATED_UCLASS_BODY()

public:
  //~ Begin AActor Interface
  virtual void BeginPlay() override;
  //~ End AActor Interface

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  USocketIOClientComponent *DirectorSocketIOComponent;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  FString DirectorAddress = "ws://localhost:3001";

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
  bool bUseWebsocketRegionPings = false;

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void Register(
    const FString &Username,
    const FString &Password,
    FRedwoodAuthUpdate OnUpdated
  );

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void Login(
    const FString &Username,
    const FString &PasswordOrToken,
    FRedwoodAuthUpdate OnUpdated
  );

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void CancelWaitingForAccountVerification();

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void ListCharacters(FRedwoodCharactersResponse OnResponse);

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void CreateCharacter(
    USIOJsonObject *Data, FRedwoodCharacterResponse OnResponse
  );

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void GetCharacterData(
    FString CharacterId, FRedwoodCharacterResponse OnResponse
  );

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void SetCharacterData(
    FString CharacterId,
    USIOJsonObject *Data,
    FRedwoodCharacterResponse OnResponse
  );

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void JoinLobby(FString Profile, FRedwoodLobbyUpdate OnUpdate);

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  FString GetMatchConnectionString(FString CharacterId);

private:
  UFUNCTION()
  void HandleDirectorConnected(
    FString SocketId, FString SessionId, bool bIsReconnection
  );

  void HandleRegionsChanged(
    const FString &Event, const TSharedPtr<FJsonValue> &Message
  );

  void InitiatePings();
  UFUNCTION()
  void HandlePingResult(FString TargetAddress, float RTT);

  TMap<FString, TSharedPtr<FDataCenterLatency>> DataCenters;
  TMap<FString, float> PingAverages;
  FTimerHandle PingTimer;
  int PingAttemptsLeft;

  void AttemptJoinLobby();
  FRedwoodLobbyUpdate OnLobbyUpdate;
  FString LobbyProfile;
  FString LobbyConnection;
  FString LobbyToken;

  FRedwoodAuthUpdate OnAccountVerified;
  FString PlayerId;
  FString AuthToken;
};

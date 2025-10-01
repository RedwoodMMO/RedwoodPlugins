// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodGameplayTags.h"
#include "RedwoodModule.h"
#include "Types/RedwoodTypes.h"

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "SocketIOFunctionLibrary.h"
#include "SocketIONative.h"

#include "RedwoodServerGameSubsystem.generated.h"

class AGameModeBase;
class URedwoodSyncItemAsset;
class URedwoodSyncComponent;

UCLASS(BlueprintType)
class REDWOOD_API URedwoodServerGameSubsystem : public UGameInstanceSubsystem {
  GENERATED_BODY()

public:
  // Begin USubsystem
  virtual void Initialize(FSubsystemCollectionBase &Collection) override;
  virtual void Deinitialize() override;
  // End USubsystem

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void CallExecCommandOnAllClients(const FString &Command);

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString RequestId;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString RealmName;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString ProxyId;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString InstanceId;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Name;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString MapId;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString ModeId;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  bool bContinuousPlay = false;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Password;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString ShortCode;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  int32 MaxPlayers = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  USIOJsonObject *Data = nullptr;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString OwnerPlayerId;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString Channel;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString ZoneName;

  // 1-based index of which instance of the zone this server is running
  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString ShardName;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString ParentProxyId;

  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FString SidecarUri;

  /**
   * Travel the specified player to a new zone transform.
   * @param bShouldStitch This is a WIP feature that you likely
   * don't have access to; leave it set to false.
   */
  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void TravelPlayerToZoneTransform(
    APlayerController *PlayerController,
    const FString &InZoneName,
    const FTransform &InTransform,
    const FString &OptionalProxyId = TEXT(""),
    bool bShouldStitch = false
  );

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void TravelPlayerToZoneSpawnName(
    APlayerController *PlayerController,
    const FString &InZoneName,
    const FString &InSpawnName = TEXT("default"),
    const FString &OptionalProxyId = TEXT("")
  );

  void FlushSync();
  void FlushPersistence();
  void FlushPlayerCharacterData(
    TArray<APlayerState *> PlayerArray, bool bForce
  );
  TSharedPtr<FJsonObject> CreatePlayerCharacterDataObject(
    APlayerState *PlayerState, bool bForce
  );
  void FlushZoneData();

  void InitialDataLoad(FRedwoodDelegate OnComplete);

  void RegisterSyncComponent(
    URedwoodSyncComponent *InComponent, bool bDelayNewSync
  );

  void PutBlob(
    const FString &Key,
    const TArray<uint8> &Value,
    FRedwoodErrorOutputDelegate OnComplete
  );
  void GetBlob(const FString &Key, FRedwoodGetBlobOutputDelegate OnComplete);

  void PutSaveGame(
    const FString &Key, USaveGame *Value, FRedwoodErrorOutputDelegate OnComplete
  );
  void GetSaveGame(
    const FString &Key, FRedwoodGetSaveGameOutputDelegate OnComplete
  );

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void RequestEngineExit(bool bForce);

private:
  TMap<FString, TSubclassOf<AGameModeBase>> GameModeClasses;
  TMap<FString, FPrimaryAssetId> Maps;
  TMap<FString, URedwoodSyncItemAsset *> SyncItemTypesByTypeId;
  TMap<FString, URedwoodSyncItemAsset *> SyncItemTypesByPrimaryAssetId;
  TMap<FString, URedwoodSyncComponent *> SyncItemComponentsById;

  void InitializeSidecar();
  void SendUpdateToSidecar();

  TSharedPtr<FSocketIONative> Sidecar;

  float UpdateSidecarRate = 3.f; // in seconds
  float UpdateSidecarLoadingRate = 0.2f; // in seconds
  FTimerHandle TimerHandle_UpdateSidecar;
  FTimerHandle TimerHandle_UpdateSidecarLoading;

  bool bIsShuttingDown = false;
  FGameplayMessageListenerHandle ListenerHandle;
  void OnShutdownMessage(FGameplayTag InChannel, const FRedwoodReason &Message);

  TSet<URedwoodSyncComponent *> DelayedNewSyncItems;
  bool bInitialDataLoaded = false;
  FRedwoodDelegate InitialDataLoadCompleteDelegate;
  void PostInitialDataLoad(TSharedPtr<FJsonObject> ZoneJsonObject);

  void UpdateSyncItem(FRedwoodSyncItem &Item);
  void UpdateSyncItemState(
    URedwoodSyncComponent *SyncItemComponent, FRedwoodSyncItemState &ItemState
  );
  void UpdateSyncItemMovement(
    URedwoodSyncComponent *SyncItemComponent,
    FRedwoodSyncItemMovement &ItemMovement
  );
  void UpdateSyncItemData(
    URedwoodSyncComponent *SyncItemComponent, USIOJsonObject *InData
  );

  void SendNewSyncItemToSidecar(URedwoodSyncComponent *InComponent);
  void SendNewSyncForPersistentItemsToSidecar();
};

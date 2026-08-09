// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodServerGameSubsystem.h"

#include "RedwoodPersistentItemsAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodFetchPersistentItemsAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Fetch Persistent Items",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodFetchPersistentItemsAsync *FetchPersistentItems(
    URedwoodServerGameSubsystem *Target,
    UObject *WorldContextObject,
    FRedwoodPersistentItemsFilter Filter
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodPersistentItemsTreeOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodServerGameSubsystem *Target;

  // Holds the parsed tree across the broadcast; the nodes are UObjects
  // and this is what keeps them from being collected before Blueprint
  // has taken its own references
  UPROPERTY()
  FRedwoodPersistentItemsTreeOutput Output;

  FRedwoodPersistentItemsFilter Filter;
};

UCLASS()
class REDWOOD_API URedwoodFetchCharacterPersistentItemsAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  // Fetches every item owned by the character along with all
  // descendant items (the full inventory tree)
  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Fetch Character Persistent Items",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodFetchCharacterPersistentItemsAsync *
  FetchCharacterPersistentItems(
    URedwoodServerGameSubsystem *Target,
    UObject *WorldContextObject,
    FString CharacterId
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodPersistentItemsTreeOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodServerGameSubsystem *Target;

  // See URedwoodFetchPersistentItemsAsync::Output
  UPROPERTY()
  FRedwoodPersistentItemsTreeOutput Output;

  FString CharacterId;
};

UCLASS()
class REDWOOD_API URedwoodSavePersistentItemsAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Save Persistent Items",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodSavePersistentItemsAsync *SavePersistentItems(
    URedwoodServerGameSubsystem *Target,
    UObject *WorldContextObject,
    TArray<FRedwoodSavePersistentItem> Items
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodPersistentItemsOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodServerGameSubsystem *Target;

  TArray<FRedwoodSavePersistentItem> Items;
};

UCLASS()
class REDWOOD_API URedwoodMovePersistentItemsToParentAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  // Moves the items into another persistent item (e.g. into a bag)
  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Move Persistent Items To Parent",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodMovePersistentItemsToParentAsync *MovePersistentItemsToParent(
    URedwoodServerGameSubsystem *Target,
    UObject *WorldContextObject,
    TArray<FString> ItemIds,
    FString NewParentId
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodPersistentItemsOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodServerGameSubsystem *Target;

  TArray<FString> ItemIds;
  FString NewParentId;
};

UCLASS()
class REDWOOD_API URedwoodMovePersistentItemsToCharacterAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  // Moves the items directly under a character (e.g. picking up)
  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Move Persistent Items To Character",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodMovePersistentItemsToCharacterAsync *
  MovePersistentItemsToCharacter(
    URedwoodServerGameSubsystem *Target,
    UObject *WorldContextObject,
    TArray<FString> ItemIds,
    FString NewOwnerCharacterId
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodPersistentItemsOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodServerGameSubsystem *Target;

  TArray<FString> ItemIds;
  FString NewOwnerCharacterId;
};

UCLASS()
class REDWOOD_API URedwoodMovePersistentItemsToWorldAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  // Moves the items into the world of this server's proxy (e.g.
  // dropping an item); the caller is responsible for spawning the
  // actor with a URedwoodSyncComponent whose RedwoodId is the item id.
  // ZoneName defaults to this server's zone when empty.
  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Move Persistent Items To World",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodMovePersistentItemsToWorldAsync *MovePersistentItemsToWorld(
    URedwoodServerGameSubsystem *Target,
    UObject *WorldContextObject,
    TArray<FString> ItemIds,
    FTransform Transform,
    FString ZoneName
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodPersistentItemsOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodServerGameSubsystem *Target;

  TArray<FString> ItemIds;
  FTransform Transform;
  FString ZoneName;
};

UCLASS()
class REDWOOD_API URedwoodDeletePersistentItemsAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  // Soft-deletes the items and their entire containment trees
  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Delete Persistent Items",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodDeletePersistentItemsAsync *DeletePersistentItems(
    URedwoodServerGameSubsystem *Target,
    UObject *WorldContextObject,
    TArray<FString> ItemIds
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodServerGameSubsystem *Target;

  TArray<FString> ItemIds;
};

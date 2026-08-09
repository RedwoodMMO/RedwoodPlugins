// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodTypesCommon.h"

#include "RedwoodTypesPersistentItems.generated.h"

// A single PersistentItem row from the realm database. Exactly one of
// ParentId, OwnerCharacterId, and OwnerProxyId is set; ProxyZoneName
// and ProxyTransform are only valid when OwnerProxyId is set.
USTRUCT(BlueprintType)
struct FRedwoodPersistentItem {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  // Matches the RedwoodTypeId in the corresponding
  // URedwoodSyncItemAsset data asset (if the item ever gets spawned in
  // the world), but item types that only live inside inventories don't
  // need a data asset at all.
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString TypeId;

  // Non-empty when this item is contained by another persistent item
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString ParentId;

  // Non-empty when this item is directly owned by a character
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString OwnerCharacterId;

  // Non-empty when this item lives in the world of a GameServerProxy
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString OwnerProxyId;

  // Only valid when OwnerProxyId is non-empty
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString ProxyZoneName;

  // Only true when OwnerProxyId is non-empty
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bHasProxyTransform = false;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FTransform ProxyTransform;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data = nullptr;
};

// One item in a fetched containment tree. The database model has no
// children column; the backend derives the hierarchy from each row's
// ParentId so a fetch hands back something you can walk directly
// instead of a flat list you have to stitch together.
//
// This is a UObject rather than a USTRUCT because a USTRUCT cannot hold
// a TArray of itself ("Struct recursion via arrays is unsupported for
// properties"); object pointers have no such restriction.
//
// Only fetches produce these. Save and move describe the rows they
// wrote, not a tree, and answer with plain FRedwoodPersistentItem.
UCLASS(BlueprintType)
class REDWOOD_API URedwoodPersistentItemNode : public UObject {
  GENERATED_BODY()

public:
  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  FRedwoodPersistentItem Item;

  // The items contained by this one, as deep as the fetch's MaxDepth
  // allowed
  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  TArray<URedwoodPersistentItemNode *> Children;

  // True when this item has contained items in the database that aren't
  // in Children because the fetch hit its depth limit. Fetch this
  // item's id again (as a ParentIds filter) to continue below it.
  UPROPERTY(BlueprintReadOnly, Category = "Redwood")
  bool bChildrenTruncated = false;
};

// Result of a save or move: the rows that were written, flat.
USTRUCT(BlueprintType)
struct FRedwoodPersistentItemsOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRedwoodPersistentItem> Items;
};

typedef TDelegate<void(const FRedwoodPersistentItemsOutput &)>
  FRedwoodPersistentItemsOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodPersistentItemsOutputDynamicDelegate,
  FRedwoodPersistentItemsOutput,
  Output
);

// Result of a fetch: the matched items as containment trees. An item is
// nested under its parent whenever that parent is also part of the same
// response, so anything whose parent is absent is a root here. Fetching
// by ParentIds therefore answers with the children as roots, and
// fetching a bag plus something inside it answers with one nested
// result rather than the same item twice.
USTRUCT(BlueprintType)
struct FRedwoodPersistentItemsTreeOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<URedwoodPersistentItemNode *> Items;
};

typedef TDelegate<void(const FRedwoodPersistentItemsTreeOutput &)>
  FRedwoodPersistentItemsTreeOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodPersistentItemsTreeOutputDynamicDelegate,
  FRedwoodPersistentItemsTreeOutput,
  Output
);

// Filter for URedwoodServerGameSubsystem::FetchPersistentItems. The
// provided id arrays are unioned together; at least one array must be
// non-empty.
USTRUCT(BlueprintType)
struct FRedwoodPersistentItemsFilter {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FString> ItemIds;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FString> OwnerCharacterIds;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FString> ParentIds;

  // Also fetch the full containment tree below every matched item
  // (e.g. bags inside bags); depth is capped by the backend
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bIncludeDescendants = false;

  // Optional descendant depth limit; 0 uses the backend maximum
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  int32 MaxDepth = 0;
};

// Input for URedwoodServerGameSubsystem::SavePersistentItems. For a new
// item (empty or unknown Id), exactly one of ParentId, OwnerCharacterId,
// or bOwnedByProxy must be set. For an existing item, leave all three
// unset to keep the current ownership, or set exactly one to change it.
USTRUCT(BlueprintType)
struct FRedwoodSavePersistentItem {
  GENERATED_BODY()

  // Leave empty to create a new item with a generated id
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString TypeId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString ParentId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString OwnerCharacterId;

  // Owns the item to this server's GameServerProxy (i.e. it lives in
  // the world); use the normal URedwoodSyncComponent flow to actually
  // spawn it
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bOwnedByProxy = false;

  // Only used with bOwnedByProxy; empty uses this server's zone
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString ProxyZoneName;

  // Only used with bOwnedByProxy
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bHasProxyTransform = false;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FTransform ProxyTransform;

  // Leave null to keep the existing data (or {} for a new item)
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data = nullptr;
};

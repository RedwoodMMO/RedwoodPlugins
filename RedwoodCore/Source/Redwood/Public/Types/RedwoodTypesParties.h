// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodTypesCommon.h"

#include "RedwoodTypesParties.generated.h"

USTRUCT(BlueprintType)
struct FRedwoodPartyInvite {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString FromPlayerId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString FromPlayerName;
};

USTRUCT(BlueprintType)
struct FRedwoodPartyMemberCharacter {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Name;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *CharacterCreatorData = nullptr;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Metadata = nullptr;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *EquippedInventory = nullptr;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *NonequippedInventory = nullptr;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Progress = nullptr;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data = nullptr;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *AbilitySystem = nullptr;
};

USTRUCT(BlueprintType)
struct FRedwoodPartyMember {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString PlayerId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Nickname;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bInstanceIdValid = false;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString InstanceId;

  // Whether the member is currently connected to the realm. An offline
  // member remains in the party (shown as away) until they reconnect, an
  // explicit leave, a leader kick, or the realm's disconnect TTL expires.
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bIsOnline = true;

  // ISO 8601 timestamp of when the member went offline; empty while the
  // member is online (bIsOnline == true).
  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString OfflineAt;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FRedwoodPartyMemberCharacter Character;
};

USTRUCT(BlueprintType) struct FRedwoodParty {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bValid = false;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString LootType;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data = nullptr;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString LeaderId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRedwoodPartyMember> Members;
};

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodPartyInvitedDynamicDelegate, FRedwoodPartyInvite, Invite
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodPartyUpdatedDynamicDelegate, FRedwoodParty, Party
);

USTRUCT(BlueprintType)
struct FRedwoodGetPartyOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FRedwoodParty Party;
};

typedef TDelegate<void(const FRedwoodGetPartyOutput &)>
  FRedwoodGetPartyOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodGetPartyOutputDynamicDelegate, FRedwoodGetPartyOutput, Data
);

USTRUCT(BlueprintType)
struct FRedwoodListPartyInvitesOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRedwoodPartyInvite> Invites;
};

typedef TDelegate<void(const FRedwoodListPartyInvitesOutput &)>
  FRedwoodListPartyInvitesOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodListPartyInvitesOutputDynamicDelegate,
  FRedwoodListPartyInvitesOutput,
  Data
);

typedef TDelegate<void(const FString &PlayerId, const FString &Emote)>
  FRedwoodPartyEmoteReceivedDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
  FRedwoodPartyEmoteReceivedDynamicDelegate,
  const FString &,
  PlayerId,
  const FString &,
  Emote
);

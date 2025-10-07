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
struct FRedwoodPartyMember {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString PlayerId;
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
  USIOJsonObject *Data;

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

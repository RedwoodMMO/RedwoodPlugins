// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodTypesCommon.h"
#include "RedwoodTypesFriends.h"

#include "RedwoodTypesGuilds.generated.h"

UENUM(BlueprintType)
enum class ERedwoodGuildInviteType : uint8 { Public, Admin, Member, Unknown };

UENUM(BlueprintType)
enum class ERedwoodGuildAndAllianceMemberState : uint8 {
  None,
  Invited,
  Member,
  Banned,
  Admin,
  Unknown
};

USTRUCT(BlueprintType)
struct FRedwoodGuildAllianceMembership {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString AllianceId;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString AllianceName;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  ERedwoodGuildAndAllianceMemberState GuildState =
    ERedwoodGuildAndAllianceMemberState::Unknown;
};

USTRUCT(BlueprintType)
struct FRedwoodGuild {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FDateTime CreatedAt;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FDateTime UpdatedAt;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Name;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  ERedwoodGuildInviteType InviteType = ERedwoodGuildInviteType::Unknown;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bListed = false;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bMembershipPublic = false;
};

USTRUCT(BlueprintType)
struct FRedwoodGuildInfo {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FRedwoodGuild Guild;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  ERedwoodGuildAndAllianceMemberState PlayerState =
    ERedwoodGuildAndAllianceMemberState::Unknown;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRedwoodGuildAllianceMembership> Alliances;
};

USTRUCT(BlueprintType)
struct FRedwoodListGuildsOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRedwoodGuildInfo> Guilds;
};

typedef TDelegate<void(const FRedwoodListGuildsOutput &)>
  FRedwoodListGuildsOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodListGuildsOutputDynamicDelegate, FRedwoodListGuildsOutput, Data
);

USTRUCT(BlueprintType)
struct FRedwoodGetGuildOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FRedwoodGuildInfo Guild;
};

typedef TDelegate<void(const FRedwoodGetGuildOutput &)>
  FRedwoodGetGuildOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodGetGuildOutputDynamicDelegate, FRedwoodGetGuildOutput, Data
);

USTRUCT(BlueprintType)
struct FRedwoodGuildPlayer {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Nickname;
};

USTRUCT(BlueprintType)
struct FRedwoodGuildPlayerMembership {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FRedwoodGuildPlayer Player;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  ERedwoodGuildAndAllianceMemberState PlayerState =
    ERedwoodGuildAndAllianceMemberState::Unknown;
};

USTRUCT(BlueprintType)
struct FRedwoodListGuildMembersOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRedwoodGuildPlayerMembership> Members;
};

typedef TDelegate<void(const FRedwoodListGuildMembersOutput &)>
  FRedwoodListGuildMembersOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodListGuildMembersOutputDynamicDelegate,
  FRedwoodListGuildMembersOutput,
  Data
);

USTRUCT(BlueprintType)
struct FRedwoodCreateGuildOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString GuildId;
};

typedef TDelegate<void(const FRedwoodCreateGuildOutput &)>
  FRedwoodCreateGuildOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodCreateGuildOutputDynamicDelegate, FRedwoodCreateGuildOutput, Data
);

USTRUCT(BlueprintType)
struct FRedwoodAlliance {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FDateTime CreatedAt;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FDateTime UpdatedAt;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Name;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bInviteOnly = false;
};

USTRUCT(BlueprintType)
struct FRedwoodListAlliancesOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRedwoodAlliance> Alliances;
};

typedef TDelegate<void(const FRedwoodListAlliancesOutput &)>
  FRedwoodListAlliancesOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodListAlliancesOutputDynamicDelegate, FRedwoodListAlliancesOutput, Data
);

USTRUCT(BlueprintType)
struct FRedwoodCreateAllianceOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString AllianceId;
};

typedef TDelegate<void(const FRedwoodCreateAllianceOutput &)>
  FRedwoodCreateAllianceOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodCreateAllianceOutputDynamicDelegate,
  FRedwoodCreateAllianceOutput,
  Data
);

USTRUCT(BlueprintType)
struct FRedwoodAllianceGuildMembership {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FRedwoodGuild Guild;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  ERedwoodGuildAndAllianceMemberState GuildState =
    ERedwoodGuildAndAllianceMemberState::Unknown;
};

USTRUCT(BlueprintType)
struct FRedwoodListAllianceGuildsOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Error;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  TArray<FRedwoodAllianceGuildMembership> Guilds;
};

typedef TDelegate<void(const FRedwoodListAllianceGuildsOutput &)>
  FRedwoodListAllianceGuildsOutputDelegate;

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodListAllianceGuildsOutputDynamicDelegate,
  FRedwoodListAllianceGuildsOutput,
  Data
);
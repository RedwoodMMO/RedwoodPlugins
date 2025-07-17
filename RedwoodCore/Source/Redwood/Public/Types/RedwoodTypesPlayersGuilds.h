// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "RedwoodTypesCommon.h"
#include "RedwoodTypesGuilds.h"
#include "RedwoodTypesPlayers.h"

#include "RedwoodTypesPlayersGuilds.generated.h"

USTRUCT(BlueprintType)
struct FRedwoodPlayerData {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Id;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FString Nickname;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  bool bSelectedGuildValid = false;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  FRedwoodGuildInfo SelectedGuild;

  UPROPERTY(BlueprintReadWrite, Category = "Redwood")
  USIOJsonObject *Data = nullptr;
};
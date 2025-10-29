// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodListGuildsAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodListGuildsAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "List Guilds",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodListGuildsAsync *ListGuilds(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    bool bOnlyPlayersGuilds
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodListGuildsOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  bool bOnlyPlayersGuilds;
};
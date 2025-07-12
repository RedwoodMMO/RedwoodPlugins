// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodSearchForGuildsAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodSearchForGuildsAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Search for Guilds",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodSearchForGuildsAsync *SearchForGuilds(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString SearchText,
    bool bIncludePartialMatches
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodListGuildsOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString SearchText;
  bool bIncludePartialMatches;
};
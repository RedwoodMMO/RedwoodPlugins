// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodGetGuildAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodGetGuildAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Get Guild",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodGetGuildAsync *GetGuild(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString GuildId
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodGetGuildOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString GuildId;
};
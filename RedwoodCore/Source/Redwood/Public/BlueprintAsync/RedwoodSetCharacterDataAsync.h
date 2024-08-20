// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodTitleGameSubsystem.h"

#include "RedwoodSetCharacterDataAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodSetCharacterDataAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Set Character Data",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodSetCharacterDataAsync *SetCharacterData(
    URedwoodTitleGameSubsystem *Target,
    UObject *WorldContextObject,
    FString CharacterId,
    FString Name,
    USIOJsonObject *Metadata,
    USIOJsonObject *EquippedInventory,
    USIOJsonObject *NonequippedInventory,
    USIOJsonObject *Data
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodGetCharacterOutputDynamicDelegate OnOutput;

  URedwoodTitleGameSubsystem *Target;

  FString CharacterId;

  FString Name;

  UPROPERTY()
  USIOJsonObject *Metadata;

  UPROPERTY()
  USIOJsonObject *EquippedInventory;

  UPROPERTY()
  USIOJsonObject *NonequippedInventory;

  UPROPERTY()
  USIOJsonObject *Data;
};
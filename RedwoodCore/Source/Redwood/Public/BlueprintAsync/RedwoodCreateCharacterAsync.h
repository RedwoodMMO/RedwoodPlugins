// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodCreateCharacterAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodCreateCharacterAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Create Character",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodCreateCharacterAsync *CreateCharacter(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString Name,
    USIOJsonObject *Metadata,
    USIOJsonObject *EquippedInventory,
    USIOJsonObject *NonequippedInventory,
    USIOJsonObject *Data
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodGetCharacterOutputDynamicDelegate OnOutput;

  URedwoodClientGameSubsystem *Target;

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
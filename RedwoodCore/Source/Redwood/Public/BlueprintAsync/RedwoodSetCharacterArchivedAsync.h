// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodSetCharacterArchivedAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodSetCharacterArchivedAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Set Character Archived",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodSetCharacterArchivedAsync *SetCharacterArchived(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString CharacterId,
    bool bArchived
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString CharacterId;
  bool bArchived;
};
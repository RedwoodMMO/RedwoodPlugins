// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodServerGameSubsystem.h"

#include "RedwoodGetBlobAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodGetBlobAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Get Blob",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodGetBlobAsync *GetBlob(
    URedwoodServerGameSubsystem *Target,
    UObject *WorldContextObject,
    FString Key
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodGetBlobOutputDynamicDelegate OnOutput;

  URedwoodServerGameSubsystem *Target;

  FString Key;
};
// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodServerGameSubsystem.h"

#include "RedwoodPutBlobAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodPutBlobAsync : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Put Blob",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodPutBlobAsync *PutBlob(
    URedwoodServerGameSubsystem *Target,
    UObject *WorldContextObject,
    FString Key,
    TArray<uint8> Blob
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodServerGameSubsystem *Target;

  FString Key;
  TArray<uint8> Blob;
};
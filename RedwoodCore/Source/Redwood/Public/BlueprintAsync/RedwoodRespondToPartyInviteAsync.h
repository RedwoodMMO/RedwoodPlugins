// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodRespondToPartyInviteAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodRespondToPartyInviteAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Respond to Party Invite",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodRespondToPartyInviteAsync *RespondToPartyInvite(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString PartyId,
    bool bAccept
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodGetPartyOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  FString PartyId;

  bool bAccept = false;
};

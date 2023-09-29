// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodTypes.h"

#include "RedwoodAuthCommon.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
  FRedwoodAuthResponse, ERedwoodAuthUpdateType, Type, FString, Message
);

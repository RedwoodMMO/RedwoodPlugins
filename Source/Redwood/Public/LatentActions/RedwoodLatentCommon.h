// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodTypes.h"

#include "RedwoodLatentCommon.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
  FRedwoodAuthResponse, ERedwoodAuthUpdateType, Type, FString, Message
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
  FRedwoodCharacterResponseLatent,
  FString,
  Error,
  FRedwoodPlayerCharacter,
  Character
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
  FRedwoodCharactersResponseLatent,
  FString,
  Error,
  TArray<FRedwoodPlayerCharacter>,
  Characters
);
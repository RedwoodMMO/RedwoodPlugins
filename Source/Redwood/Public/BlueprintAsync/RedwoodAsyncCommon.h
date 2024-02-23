// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "RedwoodTypes.h"

#include "RedwoodAsyncCommon.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodSimpleResultAsync, FString, Error
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodAuthUpdateAsync, FRedwoodAuthUpdate, Data
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodCharacterResultAsync, FRedwoodCharacterResult, Data
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodCharactersResultAsync, FRedwoodCharactersResult, Data
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodOnSocketConnectedAsync, FRedwoodSocketConnected, Result
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodServersResultAsync, FRedwoodListServers, Data
);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
  FRedwoodGetServerResultAsync, FRedwoodGetServer, Data
);

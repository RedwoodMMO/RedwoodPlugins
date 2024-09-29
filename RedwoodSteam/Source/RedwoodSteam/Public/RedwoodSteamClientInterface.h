// Copyright Incanta LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Types/RedwoodTypes.h"

#include "RedwoodSteamClientInterface.generated.h"

class URedwoodClientInterface;

UCLASS() class REDWOODSTEAM_API URedwoodSteamClientInterface : public UObject {
  GENERATED_BODY()

public:
  static void LoginWithSteam(
    URedwoodClientInterface *ClientInterface,
    FRedwoodAuthUpdateDelegate OnUpdate
  );
};

// Copyright Incanta LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Types/RedwoodTypes.h"

#include "RedwoodEOSClientInterface.generated.h"

class URedwoodClientInterface;

UCLASS() class REDWOODEOS_API URedwoodEOSClientInterface : public UObject {
  GENERATED_BODY()

public:
  static void LoginWithEOS(
    URedwoodClientInterface *ClientInterface,
    FRedwoodAuthUpdateDelegate OnUpdate
  );
};

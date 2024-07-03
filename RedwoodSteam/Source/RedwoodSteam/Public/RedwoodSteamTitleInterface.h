// Copyright Incanta LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RedwoodSteamTitleInterface.generated.h"

class URedwoodTitleInterface;

UCLASS() class REDWOODSTEAM_API URedwoodSteamTitleInterface : public UObject {
  GENERATED_BODY()

public:
  static void LoginWithSteam(
    URedwoodTitleInterface *TitleInterface, FRedwoodAuthUpdateDelegate OnUpdate
  );
};

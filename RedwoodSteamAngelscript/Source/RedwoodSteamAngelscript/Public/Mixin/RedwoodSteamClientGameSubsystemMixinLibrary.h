// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RedwoodAngelscriptForwarding.h"
#include "RedwoodClientGameSubsystem.h"
#include "RedwoodSteamClientInterface.h"

#include "RedwoodSteamClientGameSubsystemMixinLibrary.generated.h"

/**
 * ScriptMixin library adding Steam login to URedwoodClientGameSubsystem.
 *
 * URedwoodSteamClientInterface::LoginWithSteam is a static, non-UFUNCTION
 * method that takes the URedwoodClientInterface, which script cannot reach
 * (URedwoodClientGameSubsystem::GetClientInterface is not a UFUNCTION). So it
 * is mixed into the subsystem instead, mirroring URedwoodLoginWithSteamAsync:
 *
 *   URedwoodClientGameSubsystem::Get().LoginWithSteam(
 *     FRedwoodAuthUpdateDynamicCallback(this, n"HandleAuthUpdate")
 *   );
 */
UCLASS(Meta = (ScriptMixin = "URedwoodClientGameSubsystem"))
class REDWOODSTEAMANGELSCRIPT_API
  URedwoodSteamClientGameSubsystemScriptMixinLibrary : public UObject {
  GENERATED_BODY()

public:
  UFUNCTION(ScriptCallable)
  static void LoginWithSteam(
    URedwoodClientGameSubsystem *Subsystem,
    const FRedwoodAuthUpdateDynamicCallback &OnUpdate
  ) {
    URedwoodSteamClientInterface::LoginWithSteam(
      Subsystem->GetClientInterface(),
      RW_FWD(FRedwoodAuthUpdateDelegate, FRedwoodAuthUpdate, OnUpdate)
    );
  }
};

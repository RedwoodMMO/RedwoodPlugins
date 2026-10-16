// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RedwoodAngelscriptForwarding.h"
#include "RedwoodClientGameSubsystem.h"
#include "RedwoodEOSClientInterface.h"

#include "RedwoodEOSClientGameSubsystemMixinLibrary.generated.h"

/**
 * ScriptMixin library adding EOS login to URedwoodClientGameSubsystem.
 *
 * URedwoodEOSClientInterface::LoginWithEOS is a static, non-UFUNCTION method
 * that takes the URedwoodClientInterface, which script cannot reach
 * (URedwoodClientGameSubsystem::GetClientInterface is not a UFUNCTION). So it
 * is mixed into the subsystem instead, mirroring URedwoodLoginWithEOSAsync:
 *
 *   URedwoodClientGameSubsystem::Get().LoginWithEOS(
 *     FRedwoodAuthUpdateDynamicCallback(this, n"HandleAuthUpdate")
 *   );
 *
 * The EOS sign-in helpers (LoginEOS_DevAuthTool, LoginEOS_PromptAccountPortal,
 * LoginEOS_EpicGamesLauncher) are BlueprintCallable statics upstream and are
 * visible to script already.
 */
UCLASS(Meta = (ScriptMixin = "URedwoodClientGameSubsystem"))
class REDWOODEOSANGELSCRIPT_API URedwoodEOSClientGameSubsystemScriptMixinLibrary
  : public UObject {
  GENERATED_BODY()

public:
  UFUNCTION(ScriptCallable)
  static void LoginWithEOS(
    URedwoodClientGameSubsystem *Subsystem,
    const FRedwoodAuthUpdateDynamicCallback &OnUpdate
  ) {
    URedwoodEOSClientInterface::LoginWithEOS(
      Subsystem->GetClientInterface(),
      RW_FWD(FRedwoodAuthUpdateDelegate, FRedwoodAuthUpdate, OnUpdate)
    );
  }
};

// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRedwood, Log, All);

class FRedwoodModule : public IModuleInterface {
public:
  /** IModuleInterface implementation */
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;

  static void ShowNotification(
    const FString &Message,
    float Duration = 10.0f,
    bool bUseSuccessFailIcons = true,
    bool bError = true,
    const FString &SubText = TEXT("")
  );
};

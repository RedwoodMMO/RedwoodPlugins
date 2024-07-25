// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRedwoodSteam, Log, All);

class FRedwoodSteamModule : public IModuleInterface {
public:
  /** IModuleInterface implementation */
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;
};

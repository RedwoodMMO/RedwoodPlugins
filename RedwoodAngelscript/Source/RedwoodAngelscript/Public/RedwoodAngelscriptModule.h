// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class REDWOODANGELSCRIPT_API FRedwoodAngelscriptModule : public FDefaultModuleImpl {
public:
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;
};

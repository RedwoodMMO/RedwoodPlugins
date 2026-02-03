// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRedwoodEditor, Log, All);

class FRedwoodEditorModule : public IModuleInterface {
public:
  /** IModuleInterface implementation */
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;

protected:
  TSharedRef<FExtender> BindLevelMenu(
    const TSharedRef<FUICommandList> CommandList
  );
  void BuildLevelMenu(FMenuBuilder &MenuBuilder);

  void ToggleUseBackendInPIE();
  bool IsUseBackendInPIEEnabled() const;

  FText GetFallbackZoneName() const;
  void OnFallbackZoneNameChanged(const FText &Text);

private:
  FLevelEditorModule::FLevelEditorMenuExtender LevelMenuExtender;
};

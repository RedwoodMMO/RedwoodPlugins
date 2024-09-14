// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"

#include "RedwoodEditorSettings.generated.h"

UCLASS(config = EditorPerProjectUserSettings, meta = (DisplayName = "Redwood"))
class REDWOODEDITOR_API URedwoodEditorSettings : public UDeveloperSettings {
  GENERATED_BODY()

public:
  URedwoodEditorSettings() {
    CategoryName = TEXT("Plugins");
    SectionName = TEXT("Redwood");
  }

  /** Whether or not you want the client or server to try to use the backend while running PIE */
  UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
  bool bUseBackendInPIE = false;

  virtual FName GetContainerName() const override {
    return "Editor";
  }
};

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

  /** Whether or not you want the server to try to connect to the sidecar while running PIE */
  UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
  bool bConnectToSidecarInPIE = false;

  /** Whether or not you want the client to try to connect to the Redwood Backend while running PIE */
  UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
  bool bConnectClientToBackendInPIE = true;

  virtual FName GetContainerName() const override {
    return "Editor";
  }
};

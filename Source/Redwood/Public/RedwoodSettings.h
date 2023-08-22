// Copyright Incanta Games 2023. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"

#include "RedwoodSettings.generated.h"

UCLASS(config = EditorPerProjectUserSettings, meta = (DisplayName = "Redwood"))
class REDWOOD_API URedwoodSettings : public UDeveloperSettings {
  GENERATED_BODY()

public:
  URedwoodSettings() {
    CategoryName = TEXT("Plugins");
    SectionName  = TEXT("Redwood");
  }

  /** Whether or not you want the server to try to connect to the sidecar while running PIE */
  UPROPERTY(
    config,
    EditAnywhere,
    BlueprintReadWrite,
    Category = "General")
  bool bConnectToSidecarInPIE = false;

  virtual FName GetContainerName() const override {
    return "Editor";
  }
};

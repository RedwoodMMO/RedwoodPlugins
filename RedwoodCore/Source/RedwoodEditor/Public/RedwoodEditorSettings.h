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

  /**
   * When bUseBackendInPIE is false, Redwood doesn't which zone you're spawning in,
   * which will cause you to spawn at the Current Camera Location or a Player Start
   * (if one is available) instead of a URedwoodZoneSpawn actor transform. By setting
   * this FallbackZoneName, you can specify the name of the zone to use as a fallback.
   *
   * Leave this empty if you want to use the default spawning behavior when bUseBackendInPIE is false.
   */
  UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
  FString FallbackZoneName;

  virtual FName GetContainerName() const override {
    return "Editor";
  }
};

// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"

#include "RedwoodSettings.generated.h"

UCLASS(config = ProjectSettings, meta = (DisplayName = "Redwood"))
class REDWOOD_API URedwoodSettings : public UDeveloperSettings {
  GENERATED_BODY()

public:
  URedwoodSettings() {
    CategoryName = TEXT("Plugins");
    SectionName = TEXT("Redwood");
  }

  /** The full URI to connect to the Director service */
  UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
  FString DirectorUri = "ws://localhost:3001";

  /** The number of times to ping a server (using the minimum response) */
  UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
  int PingAttempts = 3;

  /** How long to wait for a ping attempt to timeout (seconds) */
  UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
  float PingTimeout = 3.f;

  /** How long to wait before refreshing ping responses (seconds) */
  UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
  float PingFrequency = 10.f;

  virtual FName GetContainerName() const override {
    return "Project";
  }
};

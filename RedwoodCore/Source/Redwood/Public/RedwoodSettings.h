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

  /** Whether or DedicatedServers automatically connect to the Redwood sidecar. */
  UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
  bool bServersAutoConnectToSidecar = true;

  /** The full URI to connect to the Director service */
  UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
  FString DirectorUri = "ws://localhost:3001";

  static FString GetDirectorUri() {
    URedwoodSettings *Settings = GetMutableDefault<URedwoodSettings>();
    FString Uri = Settings->DirectorUri;

    // read from `redwood.json` if it exists
    FString JsonPath = FPaths::ProjectDir() / TEXT("redwood.json");
    if (FPaths::FileExists(JsonPath)) {
      FString Json;
      FFileHelper::LoadFileToString(Json, *JsonPath);
      TSharedPtr<FJsonObject> JsonObject;
      TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
      if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
        if (JsonObject->HasField(TEXT("directorUri"))) {
          Uri = JsonObject->GetStringField(TEXT("directorUri"));
        }
      }
    }

    return Uri;
  }

  /**
   * If set to true, Redwood will automatically connect clients to servers
   * when they receive a request from the Realm service. This will happen
   * when the client A) requests to join a lobby session, B) requests to join
   * matchmaking, or C) is part of a party that is joining because of A or B.
   * If set to false, the URedwoodTitleGameSubsystem::OnRequestToJoinServer
   * delegate will be called instead. Clients will then need to call
   * URedwoodTitleGameSubsystem::GetConnectionConsoleCommand (which they can
   * append with additional parameters) to pass to the Execute Console Command
   * engine function. This delegate will not be fired if set to true.
   */
  UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
  bool bAutoConnectToServers = true;

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

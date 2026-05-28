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

  /**
   * When true (default), Redwood will look for a `redwood.json` file in
   * the project directory at startup and use it to override settings
   * such as `DirectorUri`. Set to false in shipped builds where you
   * never want the runtime to read from that file — for example,
   * console builds that should never be redirected away from the
   * configured director, or any deployment where you'd rather rely
   * entirely on cooked settings and accept that `DirectorUri` updates
   * require a patch.
   *
   * Independent of `PublicSigningKey`: when false, the file is ignored
   * regardless of whether it's signed. When true and `PublicSigningKey`
   * is set, the file is required to carry a valid Ed25519 signature.
   */
  UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
  bool bRedwoodJsonEnabled = true;

  /**
   * Optional base64-encoded Ed25519 public key (32 raw bytes, base64 =
   * 44 characters with padding). When set, Redwood treats any
   * untrusted-source configuration it reads (currently `redwood.json`
   * loaded from the project directory) as untrusted unless it carries
   * a valid `signature` field signed by the matching private key.
   *
   * Leaving this empty preserves the legacy behavior of trusting
   * `redwood.json` verbatim — appropriate for development; not
   * appropriate for shipped builds where local malware or a tampered
   * install could otherwise rewrite the director URI silently.
   *
   * Future signed-payload features may reuse this key, so keep it
   * generic to your project's Redwood deployment rather than scoping
   * it per config file.
   */
  UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
  FString PublicSigningKey;

  static FString GetDirectorUri();

  /**
   * If set to true, Redwood will automatically connect clients to servers
   * when they receive a request from the Realm service. This will happen
   * when the client A) requests to join a lobby session, B) requests to join
   * matchmaking, or C) is part of a party that is joining because of A or B.
   * If set to false, the URedwoodClientGameSubsystem::OnRequestToJoinServer
   * delegate will be called instead. Clients will then need to call
   * URedwoodClientGameSubsystem::GetConnectionConsoleCommand (which they can
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

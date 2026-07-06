// Copyright Incanta Games. All Rights Reserved.

using UnrealBuildTool;

public class Redwood : ModuleRules {
  public Redwood(ReadOnlyTargetRules Target) : base(Target) {
    PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

    PublicIncludePaths.AddRange(
      new string[] {
      }
    );

    PrivateIncludePaths.AddRange(
      new string[] {
      }
    );

    PublicDependencyModuleNames.AddRange(
      new string[] {
        "Core",
        "ApplicationCore",
        "GameplayTags",
        "SocketIOClient",
        "SIOJson",
        "GameplayMessageRuntime",
        "DeveloperSettings",
        "Json",
      }
    );

    PrivateDependencyModuleNames.AddRange(
      new string[] {
        "CoreUObject",
        "CoreOnline",
        "Engine",
        // Push-model replication (MARK_PROPERTY_DIRTY) lives in NetCore.
        "NetCore",
        "LatencyChecker",
        "JsonUtilities",
        "Slate",
        "SlateCore",
        // Ed25519 verification for signed config files (e.g. redwood.json)
        // when URedwoodSettings::PublicSigningKey is set.
        "OpenSSL",
      }
    );

    if (Target.Type == TargetType.Editor) {
      PrivateDependencyModuleNames.AddRange(
        new string[] {
          "RedwoodEditor",
        }
      );
    }

    DynamicallyLoadedModuleNames.AddRange(
      new string[] {
      }
    );
  }
}

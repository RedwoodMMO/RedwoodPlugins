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
        "JsonModern",
      }
    );

    PrivateDependencyModuleNames.AddRange(
      new string[] {
        "CoreUObject",
        "CoreOnline",
        "Engine",
        "LatencyChecker",
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

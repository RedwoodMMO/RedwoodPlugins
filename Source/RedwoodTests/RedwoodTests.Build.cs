// Copyright Incanta Games. All Rights Reserved.

using UnrealBuildTool;

public class RedwoodTests : ModuleRules {
  public RedwoodTests(ReadOnlyTargetRules Target) : base(Target) {
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
      }
    );

    PrivateDependencyModuleNames.AddRange(
      new string[] {
        "CoreUObject",
        "Engine",
        "Redwood",
      }
    );

    DynamicallyLoadedModuleNames.AddRange(
      new string[] {
      }
    );
  }
}

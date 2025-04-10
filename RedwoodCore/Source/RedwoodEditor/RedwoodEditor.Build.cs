// Copyright Incanta Games. All Rights Reserved.

using UnrealBuildTool;

public class RedwoodEditor : ModuleRules {
  public RedwoodEditor(ReadOnlyTargetRules Target) : base(Target) {
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
        "DeveloperSettings",
      }
    );


    PrivateDependencyModuleNames.AddRange(
      new string[] {
        "CoreUObject",
        "Engine",
        "HTTP",
      }
    );


    DynamicallyLoadedModuleNames.AddRange(
      new string[] {
      }
    );
  }
}

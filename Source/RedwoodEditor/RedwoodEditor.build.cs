// Copyright Incanta Games 2023. All Rights Reserved.

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
      }
    );


    PrivateDependencyModuleNames.AddRange(
      new string[] {
        "CoreUObject",
        "Engine",
      }
    );


    DynamicallyLoadedModuleNames.AddRange(
      new string[] {
      }
    );
  }
}

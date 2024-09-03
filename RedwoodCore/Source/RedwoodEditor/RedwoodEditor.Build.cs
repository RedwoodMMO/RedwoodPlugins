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

    // get RW_VERSION environment variable
    string? RW_VERSION = System.Environment.GetEnvironmentVariable("RW_VERSION");
    if (RW_VERSION != null) {
      PublicDefinitions.Add("RW_VERSION=" + RW_VERSION + "");
    }

    PublicDefinitions.AddRange(
      new string[] {
        "RW_SEND_ANALYTICS=1",
      }
    );
  }
}

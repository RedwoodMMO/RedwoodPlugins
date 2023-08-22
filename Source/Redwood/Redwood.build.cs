// Copyright Incanta Games 2023. All Rights Reserved.

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
        "Engine",
      }
    );


    DynamicallyLoadedModuleNames.AddRange(
      new string[] {
      }
    );
  }
}

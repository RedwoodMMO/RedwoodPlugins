// Copyright Incanta Games. All Rights Reserved.

using UnrealBuildTool;

public class RedwoodGAS : ModuleRules {
  public RedwoodGAS(ReadOnlyTargetRules Target) : base(Target) {
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
        "Redwood",
        "SocketIOClient",
        "GameplayAbilities",
        "Json",
      }
    );

    PrivateDependencyModuleNames.AddRange(
      new string[] {
        "CoreUObject",
        "Engine",
        "JsonUtilities",
        "GameplayTags",
        "GameplayTasks",
        "SIOJson",
      }
    );

    DynamicallyLoadedModuleNames.AddRange(
      new string[] {
      }
    );
  }
}

// Copyright Incanta Games. All Rights Reserved.

using UnrealBuildTool;

public class RedwoodEOS : ModuleRules {
  public RedwoodEOS(ReadOnlyTargetRules Target) : base(Target) {
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
        "DeveloperSettings",
        "Redwood",
      }
    );

    PrivateDependencyModuleNames.AddRange(
      new string[] {
        "CoreUObject",
        "Engine",
        "OnlineSubsystem",
        "PacketHandler",
      }
    );

    DynamicallyLoadedModuleNames.AddRange(
      new string[] {
      }
    );
  }
}

// Copyright Incanta Games. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

namespace UnrealBuildTool.Rules {
  // A separate plugin rather than a module of RedwoodAngelscript, so that
  // enabling the core bindings does not force-enable RedwoodSteam. See
  // RedwoodChatAngelscript.Build.cs for the full reasoning.
  public class RedwoodSteamAngelscript : ModuleRules {
    public RedwoodSteamAngelscript(ReadOnlyTargetRules Target) : base(Target) {
      PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

      PublicIncludePaths.AddRange(
        new string[] {
          ModuleDirectory + "/Public",
        }
      );

      PublicDependencyModuleNames.AddRange(
        new string[] {
          "Core",
          "CoreUObject",
          "Engine",

          "AngelscriptCode",

          "Redwood",
          "RedwoodSteam",

          // For the RW_FWD* forwarding macros and the RedwoodCore callback
          // types (FRedwoodAuthUpdateDynamicCallback).
          "RedwoodAngelscript",
        }
      );
    }
  }
}

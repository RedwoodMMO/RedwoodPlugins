// Copyright Incanta Games. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

namespace UnrealBuildTool.Rules {
  public class RedwoodAngelscript : ModuleRules {
    public RedwoodAngelscript(ReadOnlyTargetRules Target) : base(Target) {
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

          // Provides UFUNCTION(ScriptCallable) and the ScriptMixin class
          // specifier. This dependency is the entire reason these bindings
          // live in their own plugin rather than in RedwoodCore: adding it
          // there would force Angelscript onto every consuming project.
          "AngelscriptCode",

          "Redwood",

          // FRedwoodCreateProxyInput and the character/party data setters
          // take USIOJsonObject.
          "SIOJson",
        }
      );
    }
  }
}

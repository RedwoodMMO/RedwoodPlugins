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

          // UFUNCTION(ScriptCallable) and the ScriptMixin metadata come from
          // the Angelscript engine fork's UHT rather than from this module,
          // but the bindings are meaningless without Angelscript. That is
          // why they live in their own plugin rather than in RedwoodCore:
          // putting them there would force Angelscript onto every consuming
          // project.
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

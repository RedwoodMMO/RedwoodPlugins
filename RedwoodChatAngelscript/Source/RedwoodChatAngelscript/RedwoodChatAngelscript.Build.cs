// Copyright Incanta Games. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

namespace UnrealBuildTool.Rules {
  // This ships as its own PLUGIN, not merely its own module, and that
  // distinction is load-bearing. A .uplugin dependency written as
  // {"Name": "RedwoodChat", "Enabled": true} force-enables RedwoodChat even in
  // a project that explicitly disabled it, and adding "Optional": true does not
  // change that: Optional only suppresses the error when the plugin is absent
  // from disk. Because every module listed in a .uplugin is built whenever that
  // plugin is enabled, the only way a project can take the core Redwood
  // bindings WITHOUT also taking RedwoodChat is for these bindings to live in a
  // separate, separately-enabled plugin. Verified by building with RedwoodChat
  // disabled and confirming it is not pulled back in.
  public class RedwoodChatAngelscript : ModuleRules {
    public RedwoodChatAngelscript(ReadOnlyTargetRules Target) : base(Target) {
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
          "RedwoodChat",

          // For the RW_FWD* forwarding macros.
          "RedwoodAngelscript",
        }
      );
    }
  }
}

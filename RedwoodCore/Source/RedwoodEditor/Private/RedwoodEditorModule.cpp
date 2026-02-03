// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodEditorModule.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "RedwoodEditorSettings.h"

#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "FRedwoodEditorModule"

DEFINE_LOG_CATEGORY(LogRedwoodEditor);

void FRedwoodEditorModule::StartupModule() {
  // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
  FLevelEditorModule &LevelEditorModule =
    FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
  LevelMenuExtender = FLevelEditorModule::FLevelEditorMenuExtender::CreateRaw(
    this, &FRedwoodEditorModule::BindLevelMenu
  );
  auto &MenuExtenders =
    LevelEditorModule.GetAllLevelEditorToolbarPlayMenuExtenders();
  MenuExtenders.Add(LevelMenuExtender);
}

void FRedwoodEditorModule::ShutdownModule() {
  // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
  // we call this function before unloading the module.
}

TSharedRef<FExtender> FRedwoodEditorModule::BindLevelMenu(
  const TSharedRef<FUICommandList> CommandList
) {
  TSharedRef<FExtender> Extender(new FExtender());
  Extender->AddMenuExtension(
    "LevelEditorPlayInWindowNetwork",
    EExtensionHook::Before,
    CommandList,
    FMenuExtensionDelegate::CreateRaw(
      this, &FRedwoodEditorModule::BuildLevelMenu
    )
  );
  return Extender;
}

void FRedwoodEditorModule::BuildLevelMenu(FMenuBuilder &MenuBuilder) {
  // add a section "Redwood Options"
  MenuBuilder.BeginSection(
    "RedwoodOptions", LOCTEXT("RedwoodOptions", "Redwood Options")
  );

  FUIAction Action(
    FExecuteAction::CreateRaw(
      this, &FRedwoodEditorModule::ToggleUseBackendInPIE
    ),
    FCanExecuteAction(),
    FIsActionChecked::CreateRaw(
      this, &FRedwoodEditorModule::IsUseBackendInPIEEnabled
    )
  );
  MenuBuilder.AddMenuEntry(
    LOCTEXT("UseBackendInPIE", "Use Backend in PIE"),
    LOCTEXT(
      "UseBackendInPIEDesc",
      "Toggle whether to use the backend in Play In Editor mode."
    ),
    FSlateIcon(),
    Action,
    NAME_None,
    EUserInterfaceActionType::ToggleButton
  );

  // add an option for the FString FallbackZoneName. this should
  // show an editable text box, not a toggle
  MenuBuilder.AddWidget(
    SNew(SEditableTextBox)
      .Text_Raw(this, &FRedwoodEditorModule::GetFallbackZoneName)
      .OnTextChanged_Raw(
        this, &FRedwoodEditorModule::OnFallbackZoneNameChanged
      ),
    LOCTEXT("FallbackZoneName", "Fallback Zone Name"),
    false,
    true,
    LOCTEXT(
      "FallbackZoneNameDesc",
      "When not using backend in PIE, this emulates spawning in a specific zone name without prior location data. Leave empty to spawn at above setting."
    )
  );

  MenuBuilder.EndSection();
}

void FRedwoodEditorModule::ToggleUseBackendInPIE() {
  URedwoodEditorSettings *Settings =
    GetMutableDefault<URedwoodEditorSettings>();
  Settings->bUseBackendInPIE = !Settings->bUseBackendInPIE;
  Settings->SaveConfig();
}

bool FRedwoodEditorModule::IsUseBackendInPIEEnabled() const {
  const URedwoodEditorSettings *Settings = GetDefault<URedwoodEditorSettings>();
  return Settings->bUseBackendInPIE;
}

FText FRedwoodEditorModule::GetFallbackZoneName() const {
  const URedwoodEditorSettings *Settings = GetDefault<URedwoodEditorSettings>();
  return FText::FromString(Settings->FallbackZoneName);
}

void FRedwoodEditorModule::OnFallbackZoneNameChanged(const FText &Text) {
  URedwoodEditorSettings *Settings =
    GetMutableDefault<URedwoodEditorSettings>();
  Settings->FallbackZoneName = Text.ToString();
  Settings->SaveConfig();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRedwoodEditorModule, RedwoodEditor)

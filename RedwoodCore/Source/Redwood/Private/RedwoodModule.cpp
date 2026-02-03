// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodModule.h"

#if WITH_EDITOR
  #include "Framework/Notifications/NotificationManager.h"
  #include "Widgets/Notifications/SNotificationList.h"
#endif

#define LOCTEXT_NAMESPACE "FRedwoodModule"

DEFINE_LOG_CATEGORY(LogRedwood);

void FRedwoodModule::StartupModule() {
  // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FRedwoodModule::ShutdownModule() {
  // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
  // we call this function before unloading the module.
}

void FRedwoodModule::ShowNotification(
  const FString &Message,
  float Duration,
  bool bUseSuccessFailIcons,
  bool bError,
  const FString &SubText
) {
  if (bError) {
    UE_LOG(LogRedwood, Error, TEXT("%s %s"), *Message, *SubText);
  } else {
    UE_LOG(LogRedwood, Log, TEXT("%s %s"), *Message, *SubText);
  }

#if WITH_EDITOR
  FText PrefixedMessage = FText::Format(
    NSLOCTEXT("RedwoodErrorPopup", "PrefixedMessage", "Redwood: {0}"),
    FText::FromString(Message)
  );
  FNotificationInfo Info(PrefixedMessage);
  Info.ExpireDuration = Duration;
  Info.bUseSuccessFailIcons = bUseSuccessFailIcons;
  if (!SubText.IsEmpty()) {
    Info.SubText = FText::FromString(SubText);
  }
  TSharedPtr<SNotificationItem> CompileNotification =
    FSlateNotificationManager::Get().AddNotification(Info);
  CompileNotification->SetCompletionState(
    bError ? SNotificationItem::CS_Fail : SNotificationItem::CS_Success
  );
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRedwoodModule, Redwood)

// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodEditorModule.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#define LOCTEXT_NAMESPACE "FRedwoodEditorModule"

DEFINE_LOG_CATEGORY(LogRedwoodEditor);

#define RW_STRINGIZE(x) #x
#define RW_MACRO_TO_STRING(x) RW_STRINGIZE(x)

#ifndef RW_VERSION
  #define RW_VERSION source
#endif

void FRedwoodEditorModule::StartupModule() {
  // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

  // Make an analytics request to track Redwood users
#if RW_SEND_ANALYTICS
  TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest =
    FHttpModule::Get().CreateRequest();

  // Download the installer for the suggested IDE
  HttpRequest->SetVerb(TEXT("GET"));

  FString URLBase = TEXT("https://license.redwoodmmo.com/engine?page=");
  FString URLVersion = RW_MACRO_TO_STRING(RW_VERSION);
  FString URL = URLBase + URLVersion;
  HttpRequest->SetURL(URL);
  HttpRequest->ProcessRequest();
#endif
}

void FRedwoodEditorModule::ShutdownModule() {
  // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
  // we call this function before unloading the module.
}

#undef RW_MACRO_TO_STRING

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRedwoodEditorModule, RedwoodEditor)

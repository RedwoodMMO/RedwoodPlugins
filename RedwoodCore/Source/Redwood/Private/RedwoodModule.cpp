// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodModule.h"

#define LOCTEXT_NAMESPACE "FRedwoodModule"

DEFINE_LOG_CATEGORY(LogRedwood);

void FRedwoodModule::StartupModule() {
  // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FRedwoodModule::ShutdownModule() {
  // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
  // we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRedwoodModule, Redwood)

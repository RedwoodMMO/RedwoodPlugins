// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodTestsModule.h"
#include "Modules/ModuleManager.h"

static const FName EditorTestsTabName("RedwoodTests");

#define LOCTEXT_NAMESPACE "FRedwoodTestsModule"

void FRedwoodTestsModule::StartupModule() {}

void FRedwoodTestsModule::ShutdownModule() {}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRedwoodTestsModule, RedwoodTests)

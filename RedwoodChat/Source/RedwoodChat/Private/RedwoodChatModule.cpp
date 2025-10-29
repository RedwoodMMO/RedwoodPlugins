// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodChatModule.h"

#define LOCTEXT_NAMESPACE "FRedwoodChatModule"

DEFINE_LOG_CATEGORY(LogRedwoodChat);

void FRedwoodChatModule::StartupModule() {
  //
}

void FRedwoodChatModule::ShutdownModule() {
  //
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRedwoodChatModule, RedwoodChat)

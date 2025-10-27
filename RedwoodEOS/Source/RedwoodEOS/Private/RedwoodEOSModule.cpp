// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodEOSModule.h"
#include "PacketHandler.h"

#define LOCTEXT_NAMESPACE "FRedwoodEOSModule"

DEFINE_LOG_CATEGORY(LogRedwoodEOS);

void FRedwoodEOSModule::StartupModule() {
  PacketHandler::GetAddComponentByNameDelegate().BindLambda([](TArray<FString>
                                                                 &Names) {
    UE_LOG(
      LogRedwoodEOS,
      Log,
      TEXT(
        "Removing SteamAuthComponentModuleInterface PacketHandler; it shouldn't be running for a match but the config is necessary for Steam Auth to work in the main menu."
      )
    );
    Names.Remove(TEXT("OnlineSubsystemSteam.SteamAuthComponentModuleInterface")
    );
  });
}

void FRedwoodEOSModule::ShutdownModule() {
  //
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRedwoodEOSModule, RedwoodEOS)

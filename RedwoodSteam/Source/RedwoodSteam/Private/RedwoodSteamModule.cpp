// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodSteamModule.h"
#include "PacketHandler.h"

#define LOCTEXT_NAMESPACE "FRedwoodSteamModule"

DEFINE_LOG_CATEGORY(LogRedwoodSteam);

void FRedwoodSteamModule::StartupModule() {
  PacketHandler::GetAddComponentByNameDelegate().BindLambda([](TArray<FString>
                                                                 &Names) {
    UE_LOG(
      LogRedwoodSteam,
      Log,
      TEXT(
        "Removing SteamAuthComponentModuleInterface PacketHandler; it shouldn't be running for a match but the config is necessary for Steam Auth to work in the main menu."
      )
    );
    Names.Remove(TEXT("OnlineSubsystemSteam.SteamAuthComponentModuleInterface")
    );
  });
}

void FRedwoodSteamModule::ShutdownModule() {
  //
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRedwoodSteamModule, RedwoodSteam)

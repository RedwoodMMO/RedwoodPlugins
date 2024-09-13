// Copyright Incanta LLC. All Rights Reserved.

#include "RedwoodSteamClientInterface.h"
#include "RedwoodSteamModule.h"

#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystem.h"
#include "RedwoodClientInterface.h"

void URedwoodSteamClientInterface::LoginWithSteam(
  URedwoodClientInterface *ClientInterface, FRedwoodAuthUpdateDelegate OnUpdate
) {
  IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
  if (OnlineSub) {
    UE_LOG(LogRedwoodSteam, Log, TEXT("Steam subsystem loaded"));
    OnlineSub->GetIdentityInterface()->GetLinkedAccountAuthToken(
      0,
      FString(TEXT("WebAPI:redwood")),
      IOnlineIdentity::FOnGetLinkedAccountAuthTokenCompleteDelegate::
        CreateLambda([ClientInterface, OnUpdate](
                       int32 LocalUserNum,
                       bool bSuccessful,
                       const FExternalAuthToken &Token
                     ) {
          if (bSuccessful && Token.HasTokenString()) {
            ClientInterface->Login(
              "steam", // todo?
              Token.TokenString,
              "steam",
              false,
              OnUpdate
            );
          } else {
            UE_LOG(LogRedwoodSteam, Error, TEXT("Failed to get Steam token"));
          }
        })
    );
  } else {
    UE_LOG(LogRedwoodSteam, Error, TEXT("Steam subsystem not loaded"));
  }
}

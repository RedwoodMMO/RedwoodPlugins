// Copyright Incanta LLC. All Rights Reserved.

#include "RedwoodEOSClientInterface.h"
#include "RedwoodEOSModule.h"

#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystem.h"
#include "RedwoodClientInterface.h"

void URedwoodEOSClientInterface::LoginWithEOS(
  URedwoodClientInterface *ClientInterface, FRedwoodAuthUpdateDelegate OnUpdate
) {
  IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
  if (OnlineSub) {
    UE_LOG(LogRedwoodEOS, Log, TEXT("Steam subsystem loaded"));
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
            UE_LOG(LogRedwoodEOS, Error, TEXT("Failed to get Steam token"));
          }
        })
    );
  } else {
    UE_LOG(LogRedwoodEOS, Error, TEXT("Steam subsystem not loaded"));
  }
}

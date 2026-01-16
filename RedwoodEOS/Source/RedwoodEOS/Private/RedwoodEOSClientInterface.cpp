// Copyright Incanta LLC. All Rights Reserved.

#include "RedwoodEOSClientInterface.h"
#include "RedwoodEOSModule.h"

#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystem.h"
#include "RedwoodClientInterface.h"

bool URedwoodEOSClientInterface::LoginEOS_DevAuthTool(
  FString Endpoint, FString CredentialName
) {
  IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get(EOS_SUBSYSTEM);
  if (OnlineSub) {
    FOnlineAccountCredentials Credentials;
    Credentials.Type = TEXT("Developer");
    Credentials.Id = Endpoint;
    Credentials.Token = CredentialName;

    if (!OnlineSub->GetIdentityInterface()->Login(0, Credentials)) {
      return false;
    }

    return true;
  } else {
    UE_LOG(LogRedwoodEOS, Error, TEXT("EOS subsystem not loaded"));
    return false;
  }
}

bool URedwoodEOSClientInterface::LoginEOS_PromptAccountPortal() {
  IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get(EOS_SUBSYSTEM);
  if (OnlineSub) {
    FOnlineAccountCredentials Credentials;
    Credentials.Type = TEXT("AccountPortal");

    if (!OnlineSub->GetIdentityInterface()->Login(0, Credentials)) {
      return false;
    }

    return true;
  } else {
    UE_LOG(LogRedwoodEOS, Error, TEXT("EOS subsystem not loaded"));
    return false;
  }
}

bool URedwoodEOSClientInterface::LoginEOS_EpicGamesLauncher() {
  IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get(EOS_SUBSYSTEM);
  if (OnlineSub) {
    FOnlineAccountCredentials Credentials;
    Credentials.Type = TEXT("ExchangeCode");

    if (!OnlineSub->GetIdentityInterface()->Login(0, Credentials)) {
      return false;
    }

    return true;
  } else {
    UE_LOG(LogRedwoodEOS, Error, TEXT("EOS subsystem not loaded"));
    return false;
  }
}

void URedwoodEOSClientInterface::LoginWithEOS(
  URedwoodClientInterface *ClientInterface, FRedwoodAuthUpdateDelegate OnUpdate
) {
  IOnlineSubsystem *OnlineSub = IOnlineSubsystem::Get(EOS_SUBSYSTEM);
  if (OnlineSub) {
    ELoginStatus::Type LoginStatus =
      OnlineSub->GetIdentityInterface()->GetLoginStatus(0);

    if (LoginStatus != ELoginStatus::LoggedIn) {
      FString Error = TEXT("You must be logged in to EOS before calling the "
                           "URedwoodEOSClientInterface::LoginWithEOS function");

      FRedwoodAuthUpdate Output;
      Output.Type = ERedwoodAuthUpdateType::Error;
      Output.Message = Error;
      OnUpdate.ExecuteIfBound(Output);
      return;
    }

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
              "epic", // todo username?
              Token.TokenString,
              "epic",
              false,
              OnUpdate
            );
          } else {
            UE_LOG(LogRedwoodEOS, Error, TEXT("Failed to get EOS token"));
          }
        })
    );
  } else {
    UE_LOG(LogRedwoodEOS, Error, TEXT("EOS subsystem not loaded"));
  }
}

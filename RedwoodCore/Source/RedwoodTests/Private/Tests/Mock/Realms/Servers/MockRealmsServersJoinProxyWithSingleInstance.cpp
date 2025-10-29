// Copyright Incanta Games. All Rights Reserved.

#include "../../../../TestCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FMockRealmsServersJoinProxyWithSingleInstance,
  "Redwood.Mock.Realms.Servers.JoinProxyWithSingleInstance",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
);

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(
  FMockRealmsServersJoinProxyWithSingleInstanceInitialize
);
void FMockRealmsServersJoinProxyWithSingleInstanceInitialize::Initialize() {
  Redwood->InitializeDirectorConnection(
    FRedwoodSocketConnectedDelegate::CreateLambda(
      [this](const FRedwoodSocketConnected &Result) {
        CurrentTest->TestEqual(
          TEXT("Director Connection Success"), Result.Error, TEXT("")
        );

        Redwood->Login(
          "user",
          "password",
          "local",
          false,
          FRedwoodAuthUpdateDelegate::CreateLambda(
            [this](const FRedwoodAuthUpdate &AuthResult) {
              Redwood->InitializeConnectionForFirstRealm(
                FRedwoodSocketConnectedDelegate::CreateLambda(
                  [this](FRedwoodSocketConnected Output) {
                    CurrentTest->TestEqual(
                      TEXT("Realm Connection Success"), Output.Error, TEXT("")
                    );
                    Context->bIsCurrentTestComplete = true;
                  }
                )
              );
            }
          )
        );
      }
    )
  );
}

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(
  FMockRealmsServersJoinProxyWithSingleInstanceRun
);
void FMockRealmsServersJoinProxyWithSingleInstanceRun::Initialize() {
  Redwood->SetSelectedCharacter(TEXT("mock-character-id"));
  Redwood->JoinProxyWithSingleInstance(
    FString("mock-proxy-id"),
    FString(),
    FRedwoodJoinServerOutputDelegate::CreateLambda(
      [this](const FRedwoodJoinServerOutput &Output) {
        CurrentTest->TestEqual(
          TEXT("returns correct error"),
          Output.Error,
          TEXT("mock-server-get-error")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct connection"),
          Output.ConnectionUri,
          TEXT("mock-connection")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct token"), Output.Token, TEXT("mock-token")
        );

        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

bool FMockRealmsServersJoinProxyWithSingleInstance::RunTest(
  const FString &Parameters
) {
  URedwoodClientInterface *Redwood = NewObject<URedwoodClientInterface>();
  UAsyncTestContext *Context = NewObject<UAsyncTestContext>();

  Redwood->AddToRoot();
  Context->AddToRoot();

  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsServersJoinProxyWithSingleInstanceInitialize(Redwood, Context, 0)
  );
  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsServersJoinProxyWithSingleInstanceRun(Redwood, Context, 1)
  );

  ADD_LATENT_AUTOMATION_COMMAND(FWaitForEnd(Redwood, Context, 2));

  Context->Start();

  return true;
}

// Copyright Incanta Games. All Rights Reserved.

#include "../../../../TestCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FMockRealmsServersCreateProxy,
  "Redwood.Mock.Realms.Servers.CreateProxy",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
);

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsServersCreateProxyInitialize
);
void FMockRealmsServersCreateProxyInitialize::Initialize() {
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

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsServersCreateProxyRun);
void FMockRealmsServersCreateProxyRun::Initialize() {
  FRedwoodCreateProxyInput Parameters;

  Parameters.Name = "mock-server-name";
  Parameters.Region = "mock-server-region";
  Parameters.ModeId = "mock-server-mode-id";
  Parameters.MapId = "mock-server-map-id";

  Redwood->CreateProxy(
    false,
    Parameters,
    FRedwoodCreateProxyOutputDelegate::CreateLambda(
      [this](const FRedwoodCreateProxyOutput &Output) {
        CurrentTest->TestEqual(
          TEXT("returns correct error"),
          Output.Error,
          TEXT("mock-server-create-error")
        );
        CurrentTest->TestEqual(
          TEXT("returns correct reference"),
          Output.ProxyReference,
          TEXT("mock-proxy-id")
        );

        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

bool FMockRealmsServersCreateProxy::RunTest(const FString &Parameters) {
  URedwoodClientInterface *Redwood = NewObject<URedwoodClientInterface>();
  UAsyncTestContext *Context = NewObject<UAsyncTestContext>();

  Redwood->AddToRoot();
  Context->AddToRoot();

  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsServersCreateProxyInitialize(Redwood, Context, 0)
  );
  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsServersCreateProxyRun(Redwood, Context, 1)
  );

  ADD_LATENT_AUTOMATION_COMMAND(FWaitForEnd(Redwood, Context, 2));

  Context->Start();

  return true;
}

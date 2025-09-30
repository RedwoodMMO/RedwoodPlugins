// Copyright Incanta Games. All Rights Reserved.

#include "../../../../TestCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FMockRealmsServersStopProxy,
  "Redwood.Mock.Realms.Servers.StopProxy",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
);

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsServersStopProxyInitialize);
void FMockRealmsServersStopProxyInitialize::Initialize() {
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

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsServersStopProxyRun);
void FMockRealmsServersStopProxyRun::Initialize() {
  Redwood->StopProxy(
    FString("mock-proxy-id"),
    FRedwoodErrorOutputDelegate::CreateLambda([this](const FString &Error) {
      CurrentTest->TestEqual(
        TEXT("returns correct error"), Error, TEXT("mock-server-stop-error")
      );

      Context->bIsCurrentTestComplete = true;
    })
  );
}

bool FMockRealmsServersStopProxy::RunTest(const FString &Parameters) {
  URedwoodClientInterface *Redwood = NewObject<URedwoodClientInterface>();
  UAsyncTestContext *Context = NewObject<UAsyncTestContext>();

  Redwood->AddToRoot();
  Context->AddToRoot();

  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsServersStopProxyInitialize(Redwood, Context, 0)
  );
  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsServersStopProxyRun(Redwood, Context, 1)
  );

  ADD_LATENT_AUTOMATION_COMMAND(FWaitForEnd(Redwood, Context, 2));

  Context->Start();

  return true;
}

// Copyright Incanta Games. All Rights Reserved.

#include "../../../../TestCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FMockRealmsServersCreate,
  "Redwood.Mock.Realms.Servers.Create",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
);

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsServersCreateInitialize);
void FMockRealmsServersCreateInitialize::Initialize() {
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

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsServersCreateRun);
void FMockRealmsServersCreateRun::Initialize() {
  FRedwoodCreateServerInput Parameters;

  Parameters.Name = "mock-server-name";
  Parameters.Region = "mock-server-region";
  Parameters.ModeId = "mock-server-mode-id";
  Parameters.MapId = "mock-server-map-id";
  Parameters.MaxPlayers = 100;

  Redwood->CreateServer(
    false,
    Parameters,
    FRedwoodCreateServerOutputDelegate::CreateLambda(
      [this](const FRedwoodCreateServerOutput &Output) {
        CurrentTest->TestEqual(
          TEXT("returns correct error"),
          Output.Error,
          TEXT("mock-server-create-error")
        );
        CurrentTest->TestEqual(
          TEXT("returns correct reference"),
          Output.ServerReference,
          TEXT("mock-proxy-id")
        );

        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

bool FMockRealmsServersCreate::RunTest(const FString &Parameters) {
  URedwoodTitleInterface *Redwood = NewObject<URedwoodTitleInterface>();
  UAsyncTestContext *Context = NewObject<UAsyncTestContext>();

  Redwood->AddToRoot();
  Context->AddToRoot();

  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsServersCreateInitialize(Redwood, Context, 0)
  );
  ADD_LATENT_AUTOMATION_COMMAND(FMockRealmsServersCreateRun(Redwood, Context, 1)
  );

  ADD_LATENT_AUTOMATION_COMMAND(FWaitForEnd(Redwood, Context, 2));

  Context->Start();

  return true;
}

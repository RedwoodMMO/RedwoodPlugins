// Copyright Incanta Games. All Rights Reserved.

#include "../../../../TestCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FMockRealmsServersGet,
  "Redwood.Mock.Realms.Servers.Get",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
);

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsServersGetInitialize);
void FMockRealmsServersGetInitialize::Initialize() {
  Redwood->InitializeDirectorConnection(
    FRedwoodSocketConnectedDelegate::CreateLambda(
      [this](const FRedwoodSocketConnected &Result) {
        CurrentTest->TestEqual(
          TEXT("Director Connection Success"), Result.Error, TEXT("")
        );

        Redwood->Login(
          "user",
          "password",
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

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsServersGetRun);
void FMockRealmsServersGetRun::Initialize() {
  Redwood->GetServerInstance(
    FString("mock-proxy-id"),
    FString(),
    false,
    FRedwoodGetServerOutputDelegate::CreateLambda(
      [this](const FRedwoodGetServerOutput &Output) {
        CurrentTest->TestEqual(
          TEXT("returns correct error"),
          Output.Error,
          TEXT("mock-server-get-error")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct token"), Output.Token, TEXT("mock-token")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct id"),
          Output.Instance.Id,
          TEXT("mock-instance-id")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct created date"),
          Output.Instance.CreatedAt,
          FDateTime(2024, 1, 1, 0, 0, 0)
        );

        CurrentTest->TestEqual(
          TEXT("returns correct updated date"),
          Output.Instance.UpdatedAt,
          FDateTime(2024, 1, 2, 11, 42, 24)
        );

        CurrentTest->TestEqual(
          TEXT("returns correct provider id"),
          Output.Instance.ProviderId,
          TEXT("mock-provider-id")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct started date"),
          Output.Instance.StartedAt,
          FDateTime(2024, 1, 1, 0, 0, 0)
        );

        CurrentTest->TestEqual(
          TEXT("returns correct ended date"),
          (int)Output.Instance.EndedAt.GetTicks(),
          0
        );

        CurrentTest->TestEqual(
          TEXT("returns correct connection"),
          Output.Instance.Connection,
          TEXT("mock-connection")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct container id"),
          Output.Instance.ContainerId,
          TEXT("mock-container-id")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct proxy id"),
          Output.Instance.ProxyId,
          TEXT("mock-proxy-id")
        );

        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

bool FMockRealmsServersGet::RunTest(const FString &Parameters) {
  URedwoodTitleInterface *Redwood = NewObject<URedwoodTitleInterface>();
  UAsyncTestContext *Context = NewObject<UAsyncTestContext>();

  Redwood->AddToRoot();
  Context->AddToRoot();

  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsServersGetInitialize(Redwood, Context, 0)
  );
  ADD_LATENT_AUTOMATION_COMMAND(FMockRealmsServersGetRun(Redwood, Context, 1));

  ADD_LATENT_AUTOMATION_COMMAND(FWaitForEnd(Redwood, Context, 2));

  Context->Start();

  return true;
}

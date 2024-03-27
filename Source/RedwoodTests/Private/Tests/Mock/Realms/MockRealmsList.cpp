// Copyright Incanta Games. All Rights Reserved.

#include "../../../TestCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FMockRealmsList,
  "Redwood.Mock.Realms.List",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
);

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsListInitialize);
void FMockRealmsListInitialize::Initialize() {
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
            [this](const FRedwoodAuthUpdate &Result) {
              Context->bIsCurrentTestComplete = true;
            }
          )
        );
      }
    )
  );
}

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsListRun);
void FMockRealmsListRun::Initialize() {
  Redwood->ListRealms(FRedwoodListRealmsOutputDelegate::CreateLambda(
    [this](const FRedwoodListRealmsOutput &Output) {
      CurrentTest->TestEqual(
        TEXT("mocked error"), Output.Error, TEXT("mock-realm-list-error")
      );

      CurrentTest->TestFalse(
        TEXT("returns not single realm"), Output.bSingleRealm
      );

      CurrentTest->TestEqual(TEXT("returns one realm"), Output.Realms.Num(), 1);

      if (Output.Realms.Num() == 0) {
        // we need to do this because otherwise we'll crash since subsequent
        // calls are executed
        Context->bIsCurrentTestComplete = true;
        return;
      }

      CurrentTest->TestEqual(
        TEXT("returns correct realm id"),
        Output.Realms[0].Id,
        TEXT("mock-realm-id")
      );

      CurrentTest->TestEqual(
        TEXT("returns correct realm created date"),
        Output.Realms[0].CreatedAt,
        FDateTime(2024, 1, 1, 0, 0, 0)
      );

      CurrentTest->TestEqual(
        TEXT("returns correct realm created date"),
        Output.Realms[0].UpdatedAt,
        FDateTime(2024, 1, 2, 11, 42, 24)
      );

      CurrentTest->TestEqual(
        TEXT("returns correct realm name"),
        Output.Realms[0].Name,
        TEXT("mock-realm-name")
      );

      CurrentTest->TestEqual(
        TEXT("returns correct realm uri"),
        Output.Realms[0].Uri,
        TEXT("mock-realm-uri")
      );

      CurrentTest->TestEqual(
        TEXT("returns correct realm ping host"),
        Output.Realms[0].PingHost,
        TEXT("mock-realm-ping-host")
      );

      CurrentTest->TestEqual(
        TEXT("returns correct realm secret"),
        Output.Realms[0].Secret,
        TEXT("mock-realm-secret")
      );

      Context->bIsCurrentTestComplete = true;
    }
  ));
}

bool FMockRealmsList::RunTest(const FString &Parameters) {
  URedwoodTitleInterface *Redwood = NewObject<URedwoodTitleInterface>();
  UAsyncTestContext *Context = NewObject<UAsyncTestContext>();

  Redwood->AddToRoot();
  Context->AddToRoot();

  ADD_LATENT_AUTOMATION_COMMAND(FMockRealmsListInitialize(Redwood, Context, 0));
  ADD_LATENT_AUTOMATION_COMMAND(FMockRealmsListRun(Redwood, Context, 1));

  ADD_LATENT_AUTOMATION_COMMAND(FWaitForEnd(Redwood, Context, 2));

  Context->Start();

  return true;
}

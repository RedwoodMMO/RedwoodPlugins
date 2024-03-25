// Copyright Incanta Games. All Rights Reserved.

#include "../../../../TestCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FMockRealmsServersList,
  "Redwood.Mock.Realms.Servers.List",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
);

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsServersListInitialize);
void FMockRealmsServersListInitialize::Initialize() {
  Redwood->InitializeDirectorConnection(
    FRedwoodSocketConnectedDelegate::CreateLambda(
      [this](const FRedwoodSocketConnected &Result) {
        CurrentTest->TestEqual(
          TEXT("Director Connection Success"), Result.Error, TEXT("")
        );

        Redwood->Login(
          "user",
          "password",
          FRedwoodAuthUpdateDelegate::CreateLambda(
            [this](const FRedwoodAuthUpdate &Result) {
              Redwood->InitializeSingleRealmConnection(
                FRedwoodSocketConnectedDelegate::CreateLambda(
                  [this](const FRedwoodSocketConnected &RealmResult) {
                    CurrentTest->TestEqual(
                      TEXT("Single Realm Connection Success"),
                      RealmResult.Error,
                      TEXT("")
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

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsServersListRun);
void FMockRealmsServersListRun::Initialize() {
  Redwood->ListServers(
    TArray<FString>(),
    FRedwoodListServersOutputDelegate::CreateLambda(
      [this](const FRedwoodListServersOutput &Output) {
        CurrentTest->TestEqual(
          TEXT("returns correct error"),
          Output.Error,
          TEXT("mock-server-list-error")
        );

        CurrentTest->TestEqual(
          TEXT("returns 1 server"), Output.Servers.Num(), 1
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server id"),
          Output.Servers[0].Id,
          TEXT("mock-proxy-id")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server created date"),
          Output.Servers[0].CreatedAt,
          FDateTime(2024, 1, 1, 0, 0, 0)
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server updated date"),
          Output.Servers[0].UpdatedAt,
          FDateTime(2024, 1, 2, 11, 42, 24)
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server ended date"),
          (int)Output.Servers[0].EndedAt.GetTicks(),
          0
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server name"),
          Output.Servers[0].Name,
          TEXT("mock-server-name")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server region"),
          Output.Servers[0].Region,
          TEXT("mock-region")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server mode id"),
          Output.Servers[0].ModeId,
          TEXT("mock-mode-id")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server map id"),
          Output.Servers[0].MapId,
          TEXT("mock-map-id")
        );

        CurrentTest->TestTrue(
          TEXT("returns correct server public"), Output.Servers[0].bPublic
        );

        CurrentTest->TestTrue(
          TEXT("returns correct server continuous play"),
          Output.Servers[0].bContinuousPlay
        );

        CurrentTest->TestTrue(
          TEXT("returns correct server has password"),
          Output.Servers[0].bHasPassword
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server password"),
          Output.Servers[0].Password,
          TEXT("mock-password")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server short code"),
          Output.Servers[0].ShortCode,
          TEXT("mock-short-code")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server current players"),
          Output.Servers[0].CurrentPlayers,
          42
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server max players"),
          Output.Servers[0].MaxPlayers,
          100
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server max players"),
          Output.Servers[0].Data->GetStringField(TEXT("mock")),
          TEXT("data")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server max players"),
          Output.Servers[0].OwnerPlayerId,
          TEXT("mock-owner-id")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server max players"),
          Output.Servers[0].ActiveInstanceId,
          TEXT("mock-instance-id")
        );

        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

bool FMockRealmsServersList::RunTest(const FString &Parameters) {
  URedwoodTitleInterface *Redwood = NewObject<URedwoodTitleInterface>();
  UAsyncTestContext *Context = NewObject<UAsyncTestContext>();

  Redwood->AddToRoot();
  Context->AddToRoot();

  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsServersListInitialize(Redwood, Context, 0)
  );
  ADD_LATENT_AUTOMATION_COMMAND(FMockRealmsServersListRun(Redwood, Context, 1));

  ADD_LATENT_AUTOMATION_COMMAND(FWaitForEnd(Redwood, Context, 2));

  Context->Start();

  return true;
}

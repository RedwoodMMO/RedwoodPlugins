// Copyright Incanta Games. All Rights Reserved.

#include "../../../../TestCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FMockRealmsServersListProxies,
  "Redwood.Mock.Realms.Servers.ListProxies",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
);

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsServersListProxiesInitialize
);
void FMockRealmsServersListProxiesInitialize::Initialize() {
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

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsServersListProxiesRun);
void FMockRealmsServersListProxiesRun::Initialize() {
  Redwood->ListProxies(
    TArray<FString>(),
    FRedwoodListProxiesOutputDelegate::CreateLambda(
      [this](const FRedwoodListProxiesOutput &Output) {
        CurrentTest->TestEqual(
          TEXT("returns correct error"),
          Output.Error,
          TEXT("mock-server-list-error")
        );

        CurrentTest->TestEqual(
          TEXT("returns 1 server"), Output.Proxies.Num(), 1
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server id"),
          Output.Proxies[0].Id,
          TEXT("mock-proxy-id")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server created date"),
          Output.Proxies[0].CreatedAt,
          FDateTime(2024, 1, 1, 0, 0, 0)
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server updated date"),
          Output.Proxies[0].UpdatedAt,
          FDateTime(2024, 1, 2, 11, 42, 24)
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server ended date"),
          (int)Output.Proxies[0].EndedAt.GetTicks(),
          0
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server name"),
          Output.Proxies[0].Name,
          TEXT("mock-server-name")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server region"),
          Output.Proxies[0].Region,
          TEXT("mock-region")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server mode id"),
          Output.Proxies[0].ModeId,
          TEXT("mock-mode-id")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server map id"),
          Output.Proxies[0].MapId,
          TEXT("mock-map-id")
        );

        CurrentTest->TestTrue(
          TEXT("returns correct server public"), Output.Proxies[0].bPublic
        );

        CurrentTest->TestTrue(
          TEXT("returns correct server continuous play"),
          Output.Proxies[0].bContinuousPlay
        );

        CurrentTest->TestTrue(
          TEXT("returns correct server has password"),
          Output.Proxies[0].bHasPassword
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server password"),
          Output.Proxies[0].Password,
          TEXT("mock-password")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server short code"),
          Output.Proxies[0].ShortCode,
          TEXT("mock-short-code")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server current players"),
          Output.Proxies[0].CurrentPlayers,
          42
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server max players"),
          Output.Proxies[0].MaxPlayersPerShard,
          100
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server max players"),
          Output.Proxies[0].Data->GetStringField(TEXT("mock")),
          TEXT("data")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server max players"),
          Output.Proxies[0].OwnerPlayerId,
          TEXT("mock-owner-id")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server max players"),
          Output.Proxies[0].ActiveCollectionId,
          TEXT("mock-collection-id")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server num zones"),
          Output.Proxies[0].Zones.Num(),
          2
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server zone name"),
          Output.Proxies[0].Zones[0],
          TEXT("mock-zone-1")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server zone name"),
          Output.Proxies[0].Zones[1],
          TEXT("mock-zone-2")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct server numPlayersToAddInstance"),
          Output.Proxies[0].NumPlayersToAddShard,
          10
        );

        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

bool FMockRealmsServersListProxies::RunTest(const FString &Parameters) {
  URedwoodClientInterface *Redwood = NewObject<URedwoodClientInterface>();
  UAsyncTestContext *Context = NewObject<UAsyncTestContext>();

  Redwood->AddToRoot();
  Context->AddToRoot();

  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsServersListProxiesInitialize(Redwood, Context, 0)
  );
  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsServersListProxiesRun(Redwood, Context, 1)
  );

  ADD_LATENT_AUTOMATION_COMMAND(FWaitForEnd(Redwood, Context, 2));

  Context->Start();

  return true;
}

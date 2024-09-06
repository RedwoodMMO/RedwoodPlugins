// Copyright Incanta Games. All Rights Reserved.

#include "../../../TestCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FMockAuthLogin,
  "Redwood.Mock.Auth.Login",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
);

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockAuthLoginInitialize);
void FMockAuthLoginInitialize::Initialize() {
  Redwood->InitializeDirectorConnection(
    FRedwoodSocketConnectedDelegate::CreateLambda(
      [this](const FRedwoodSocketConnected &Result) {
        CurrentTest->TestEqual(
          TEXT("Director Connection Success"), Result.Error, TEXT("")
        );
        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockAuthLoginRun);
void FMockAuthLoginRun::Initialize() {
  Redwood->Login(
    "user",
    "password",
    "local",
    false,
    FRedwoodAuthUpdateDelegate::CreateLambda([this](
                                               const FRedwoodAuthUpdate &Result
                                             ) {
      CurrentTest->TestEqual(
        TEXT("Login Success"), Result.Type, ERedwoodAuthUpdateType::Success
      );
      CurrentTest->TestTrue(TEXT("Is Logged In"), Redwood->IsLoggedIn());
      CurrentTest->TestEqual(
        TEXT("Player Id"), Redwood->GetPlayerId(), TEXT("mock-login-player-id")
      );
      CurrentTest->TestEqual(TEXT("Login message"), Result.Message, TEXT(""));
      Context->bIsCurrentTestComplete = true;
    })
  );
}

bool FMockAuthLogin::RunTest(const FString &Parameters) {
  URedwoodClientInterface *Redwood = NewObject<URedwoodClientInterface>();
  UAsyncTestContext *Context = NewObject<UAsyncTestContext>();

  Redwood->AddToRoot();
  Context->AddToRoot();

  ADD_LATENT_AUTOMATION_COMMAND(FMockAuthLoginInitialize(Redwood, Context, 0));
  ADD_LATENT_AUTOMATION_COMMAND(FMockAuthLoginRun(Redwood, Context, 1));

  ADD_LATENT_AUTOMATION_COMMAND(FWaitForEnd(Redwood, Context, 2));

  Context->Start();

  return true;
}

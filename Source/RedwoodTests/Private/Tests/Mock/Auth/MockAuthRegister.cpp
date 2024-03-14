// Copyright Incanta Games. All Rights Reserved.

#include "../../../TestCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FMockAuthRegister,
  "Redwood.Mock.Auth.Register",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
);

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockAuthRegisterInitialize);
void FMockAuthRegisterInitialize::Initialize() {
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

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockAuthRegisterRegisterError);
void FMockAuthRegisterRegisterError::Initialize() {
  Redwood->Register(
    "mock-register-error",
    "password",
    FRedwoodAuthUpdateDelegate::CreateLambda([this](
                                               const FRedwoodAuthUpdate &Result
                                             ) {
      CurrentTest->TestEqual(
        TEXT("Register sends error"), Result.Type, ERedwoodAuthUpdateType::Error
      );
      CurrentTest->TestEqual(
        TEXT("Register sends error"),
        Result.Message,
        TEXT("mock-register-error")
      );
      Context->bIsCurrentTestComplete = true;
    })
  );
}

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockAuthRegisterRegisterVerifyAccount
);
void FMockAuthRegisterRegisterVerifyAccount::Initialize() {
  Redwood->Register(
    "mock-register-verify",
    "password",
    FRedwoodAuthUpdateDelegate::CreateLambda(
      [this](const FRedwoodAuthUpdate &Result) {
        CurrentTest->TestEqual(
          TEXT("Register sends verify account"),
          Result.Type,
          ERedwoodAuthUpdateType::MustVerifyAccount
        );
        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockAuthRegisterRegisterSuccess);
void FMockAuthRegisterRegisterSuccess::Initialize() {
  Redwood->Register(
    "mock-register-success",
    "password",
    FRedwoodAuthUpdateDelegate::CreateLambda(
      [this](const FRedwoodAuthUpdate &Result) {
        CurrentTest->TestEqual(
          TEXT("Register sends success"),
          Result.Type,
          ERedwoodAuthUpdateType::Success
        );
        CurrentTest->TestEqual(
          TEXT("Register sends correct player id"),
          Redwood->GetPlayerId(),
          TEXT("mock-register-player-id")
        );
        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

bool FMockAuthRegister::RunTest(const FString &Parameters) {
  URedwoodTitleInterface *Redwood = NewObject<URedwoodTitleInterface>();
  UAsyncTestContext *Context = NewObject<UAsyncTestContext>();

  Redwood->AddToRoot();
  Context->AddToRoot();

  ADD_LATENT_AUTOMATION_COMMAND(FMockAuthRegisterInitialize(Redwood, Context, 0)
  );
  ADD_LATENT_AUTOMATION_COMMAND(
    FMockAuthRegisterRegisterError(Redwood, Context, 1)
  );
  ADD_LATENT_AUTOMATION_COMMAND(
    FMockAuthRegisterRegisterVerifyAccount(Redwood, Context, 2)
  );
  ADD_LATENT_AUTOMATION_COMMAND(
    FMockAuthRegisterRegisterSuccess(Redwood, Context, 3)
  );

  ADD_LATENT_AUTOMATION_COMMAND(FWaitForEnd(Redwood, Context, 4));

  Context->Start();

  return true;
}

// Copyright Incanta Games. All Rights Reserved.

#include "../../../../TestCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FMockRealmsCharactersSet,
  "Redwood.Mock.Realms.Characters.Set",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
);

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsCharactersSetInitialize);
void FMockRealmsCharactersSetInitialize::Initialize() {
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

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsCharactersSetRun);
void FMockRealmsCharactersSetRun::Initialize() {
  USIOJsonObject *CharacterCreatorData = NewObject<USIOJsonObject>();
  CharacterCreatorData->SetNumberField(TEXT("level"), 2.0f);

  Redwood->SetCharacterData(
    TEXT("ignored-id"),
    TEXT("mock-character-new-name"),
    CharacterCreatorData,
    FRedwoodGetCharacterOutputDelegate::CreateLambda(
      [this, CharacterCreatorData](const FRedwoodGetCharacterOutput &Output) {
        CurrentTest->TestEqual(
          TEXT("returns correct error"),
          Output.Error,
          TEXT("mock-character-set-error")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct character id"),
          Output.Character.Id,
          TEXT("mock-character-id")
        );

        CurrentTest->TestEqual(
          TEXT("returns correct character created date"),
          Output.Character.CreatedAt,
          FDateTime(2024, 1, 1, 0, 0, 0)
        );

        CurrentTest->TestEqual(
          TEXT("returns correct character updated date"),
          Output.Character.UpdatedAt,
          FDateTime(2024, 1, 2, 11, 42, 24)
        );

        CurrentTest->TestEqual(
          TEXT("returns correct character name"),
          Output.Character.PlayerId,
          TEXT("mock-player-id")
        );

        CurrentTest->TestTrue(
          TEXT("returns valid CharacterCreatorData object"),
          IsValid(Output.Character.CharacterCreatorData)
        );

        CurrentTest->TestEqual(
          TEXT("returns character name"),
          Output.Character.Name,
          TEXT("mock-character-new-name")
        );

        CurrentTest->TestEqual(
          TEXT("returns character CharacterCreatorData level"),
          Output.Character.CharacterCreatorData->GetNumberField(TEXT("level")),
          CharacterCreatorData->GetNumberField(TEXT("level"))
        );

        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

bool FMockRealmsCharactersSet::RunTest(const FString &Parameters) {
  URedwoodClientInterface *Redwood = NewObject<URedwoodClientInterface>();
  UAsyncTestContext *Context = NewObject<UAsyncTestContext>();

  Redwood->AddToRoot();
  Context->AddToRoot();

  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsCharactersSetInitialize(Redwood, Context, 0)
  );
  ADD_LATENT_AUTOMATION_COMMAND(FMockRealmsCharactersSetRun(Redwood, Context, 1)
  );

  ADD_LATENT_AUTOMATION_COMMAND(FWaitForEnd(Redwood, Context, 2));

  Context->Start();

  return true;
}

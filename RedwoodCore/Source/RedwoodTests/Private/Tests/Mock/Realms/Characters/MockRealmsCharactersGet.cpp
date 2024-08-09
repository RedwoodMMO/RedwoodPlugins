// Copyright Incanta Games. All Rights Reserved.

#include "../../../../TestCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FMockRealmsCharactersGet,
  "Redwood.Mock.Realms.Characters.Get",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
);

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsCharactersGetInitialize);
void FMockRealmsCharactersGetInitialize::Initialize() {
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

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsCharactersGetRun);
void FMockRealmsCharactersGetRun::Initialize() {
  Redwood->GetCharacterData(
    TEXT("ignored-id"),
    FRedwoodGetCharacterOutputDelegate::CreateLambda(
      [this](const FRedwoodGetCharacterOutput &Output) {
        CurrentTest->TestEqual(
          TEXT("returns correct error"),
          Output.Error,
          TEXT("mock-character-get-error")
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
          TEXT("returns valid metadata object"),
          IsValid(Output.Character.Metadata)
        );

        CurrentTest->TestEqual(
          TEXT("returns character metadata name"),
          Output.Character.Name,
          TEXT("mock-character-name")
        );

        CurrentTest->TestEqual(
          TEXT("returns character metadata level"),
          Output.Character.Metadata->GetNumberField(TEXT("level")),
          1.0f
        );

        CurrentTest->TestTrue(
          TEXT("returns valid EquippedInventory object"),
          IsValid(Output.Character.EquippedInventory)
        );

        CurrentTest->TestEqual(
          TEXT("returns character equipped head item"),
          Output.Character.EquippedInventory->GetStringField(TEXT("head")),
          TEXT("mock-head-item")
        );

        TArray<USIOJsonValue *> Backpack =
          Output.Character.EquippedInventory->GetArrayField(TEXT("backpack"));

        CurrentTest->TestEqual(
          TEXT("returns character equipped backpack item"),
          Backpack[0]->AsString(),
          TEXT("mock-backpack-item")
        );

        CurrentTest->TestTrue(
          TEXT("returns valid NonequippedInventory object"),
          IsValid(Output.Character.NonequippedInventory)
        );

        TArray<USIOJsonValue *> Bank =
          Output.Character.NonequippedInventory->GetArrayField(TEXT("bank"));

        CurrentTest->TestEqual(
          TEXT("returns character bank item"),
          Bank[0]->AsString(),
          TEXT("mock-bank-item")
        );

        CurrentTest->TestTrue(
          TEXT("returns valid Data object"), IsValid(Output.Character.Data)
        );

        CurrentTest->TestEqual(
          TEXT("returns character last location"),
          Output.Character.Data->GetStringField(TEXT("lastLocation")),
          TEXT("mock-location")
        );

        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

bool FMockRealmsCharactersGet::RunTest(const FString &Parameters) {
  URedwoodTitleInterface *Redwood = NewObject<URedwoodTitleInterface>();
  UAsyncTestContext *Context = NewObject<UAsyncTestContext>();

  Redwood->AddToRoot();
  Context->AddToRoot();

  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsCharactersGetInitialize(Redwood, Context, 0)
  );
  ADD_LATENT_AUTOMATION_COMMAND(FMockRealmsCharactersGetRun(Redwood, Context, 1)
  );

  ADD_LATENT_AUTOMATION_COMMAND(FWaitForEnd(Redwood, Context, 2));

  Context->Start();

  return true;
}

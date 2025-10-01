// Copyright Incanta Games. All Rights Reserved.

#include "../../../../TestCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FMockRealmsCharactersList,
  "Redwood.Mock.Realms.Characters.List",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
);

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsCharactersListInitialize);
void FMockRealmsCharactersListInitialize::Initialize() {
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

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsCharactersListRun);
void FMockRealmsCharactersListRun::Initialize() {
  Redwood->ListCharacters(FRedwoodListCharactersOutputDelegate::CreateLambda(
    [this](const FRedwoodListCharactersOutput &Output) {
      CurrentTest->TestEqual(
        TEXT("returns correct error"),
        Output.Error,
        TEXT("mock-character-list-error")
      );

      CurrentTest->TestEqual(
        TEXT("returns 1 character"), Output.Characters.Num(), 1
      );

      CurrentTest->TestEqual(
        TEXT("returns correct character id"),
        Output.Characters[0].Id,
        TEXT("mock-character-id")
      );

      CurrentTest->TestEqual(
        TEXT("returns correct character created date"),
        Output.Characters[0].CreatedAt,
        FDateTime(2024, 1, 1, 0, 0, 0)
      );

      CurrentTest->TestEqual(
        TEXT("returns correct character updated date"),
        Output.Characters[0].UpdatedAt,
        FDateTime(2024, 1, 2, 11, 42, 24)
      );

      CurrentTest->TestEqual(
        TEXT("returns correct character name"),
        Output.Characters[0].PlayerId,
        TEXT("mock-player-id")
      );

      CurrentTest->TestTrue(
        TEXT("returns valid metadata object"),
        IsValid(Output.Characters[0].Metadata)
      );

      CurrentTest->TestEqual(
        TEXT("returns character metadata name"),
        Output.Characters[0].Name,
        TEXT("mock-character-name")
      );

      CurrentTest->TestEqual(
        TEXT("returns character metadata level"),
        Output.Characters[0].Metadata->GetNumberField(TEXT("level")),
        1.0f
      );

      CurrentTest->TestTrue(
        TEXT("returns valid EquippedInventory object"),
        IsValid(Output.Characters[0].EquippedInventory)
      );

      CurrentTest->TestEqual(
        TEXT("returns character equipped head item"),
        Output.Characters[0].EquippedInventory->GetStringField(TEXT("head")),
        TEXT("mock-head-item")
      );

      TArray<USIOJsonValue *> Backpack =
        Output.Characters[0].EquippedInventory->GetArrayField(TEXT("backpack"));

      CurrentTest->TestEqual(
        TEXT("returns character equipped backpack item"),
        Backpack[0]->AsString(),
        TEXT("mock-backpack-item")
      );

      CurrentTest->TestTrue(
        TEXT("returns valid NonequippedInventory object"),
        IsValid(Output.Characters[0].NonequippedInventory)
      );

      TArray<USIOJsonValue *> Bank =
        Output.Characters[0].NonequippedInventory->GetArrayField(TEXT("bank"));

      CurrentTest->TestEqual(
        TEXT("returns character bank item"),
        Bank[0]->AsString(),
        TEXT("mock-bank-item")
      );

      CurrentTest->TestTrue(
        TEXT("returns valid Progress object"),
        IsValid(Output.Characters[0].Progress)
      );

      CurrentTest->TestTrue(
        TEXT("returns valid Data object"), IsValid(Output.Characters[0].Data)
      );

      CurrentTest->TestEqual(
        TEXT("returns character last location"),
        Output.Characters[0].Data->GetStringField(TEXT("lastLocation")),
        TEXT("mock-location")
      );

      Context->bIsCurrentTestComplete = true;
    }
  ));
}

bool FMockRealmsCharactersList::RunTest(const FString &Parameters) {
  URedwoodClientInterface *Redwood = NewObject<URedwoodClientInterface>();
  UAsyncTestContext *Context = NewObject<UAsyncTestContext>();

  Redwood->AddToRoot();
  Context->AddToRoot();

  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsCharactersListInitialize(Redwood, Context, 0)
  );
  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsCharactersListRun(Redwood, Context, 1)
  );

  ADD_LATENT_AUTOMATION_COMMAND(FWaitForEnd(Redwood, Context, 2));

  Context->Start();

  return true;
}

// Copyright Incanta Games. All Rights Reserved.

#include "../../../../TestCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FMockRealmsCharactersCreate,
  "Redwood.Mock.Realms.Characters.Set",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
);

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsCharactersCreateInitialize);
void FMockRealmsCharactersCreateInitialize::Initialize() {
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

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FMockRealmsCharactersCreateRun);
void FMockRealmsCharactersCreateRun::Initialize() {
  USIOJsonObject *Metadata = NewObject<USIOJsonObject>();
  Metadata->SetNumberField(TEXT("level"), 2.0f);

  USIOJsonObject *EquippedInventory = NewObject<USIOJsonObject>();
  EquippedInventory->SetStringField(TEXT("head"), TEXT("mock-head-new-item"));
  TArray<USIOJsonValue *> Backpack;
  Backpack.Add(USIOJsonValue::ConstructJsonValueString(
    nullptr, TEXT("mock-backpack-new-item")
  ));
  EquippedInventory->SetArrayField(TEXT("backpack"), Backpack);

  USIOJsonObject *NonequippedInventory = NewObject<USIOJsonObject>();
  TArray<USIOJsonValue *> Bank;
  Bank.Add(
    USIOJsonValue::ConstructJsonValueString(nullptr, TEXT("mock-bank-new-item"))
  );
  NonequippedInventory->SetArrayField(TEXT("bank"), Bank);

  USIOJsonObject *Data = NewObject<USIOJsonObject>();
  Data->SetStringField(TEXT("lastLocation"), TEXT("mock-new-location"));

  Redwood->CreateCharacter(
    TEXT("mock-character-new-name"),
    Metadata,
    EquippedInventory,
    NonequippedInventory,
    Data,
    FRedwoodGetCharacterOutputDelegate::CreateLambda(
      [this, Metadata, EquippedInventory, NonequippedInventory, Data](
        const FRedwoodGetCharacterOutput &Output
      ) {
        CurrentTest->TestEqual(
          TEXT("returns correct error"),
          Output.Error,
          TEXT("mock-character-create-error")
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
          TEXT("returns character name"),
          Output.Character.Name,
          TEXT("mock-character-name")
        );

        CurrentTest->TestEqual(
          TEXT("returns character metadata level"),
          Output.Character.Metadata->GetNumberField(TEXT("level")),
          Metadata->GetNumberField(TEXT("level"))
        );

        CurrentTest->TestTrue(
          TEXT("returns valid EquippedInventory object"),
          IsValid(Output.Character.EquippedInventory)
        );

        CurrentTest->TestEqual(
          TEXT("returns character equipped head item"),
          Output.Character.EquippedInventory->GetStringField(TEXT("head")),
          EquippedInventory->GetStringField(TEXT("head"))
        );

        TArray<USIOJsonValue *> Backpack =
          Output.Character.EquippedInventory->GetArrayField(TEXT("backpack"));

        CurrentTest->TestEqual(
          TEXT("returns character equipped backpack item"),
          Backpack[0]->AsString(),
          EquippedInventory->GetArrayField(TEXT("backpack"))[0]->AsString()
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
          NonequippedInventory->GetArrayField(TEXT("bank"))[0]->AsString()
        );

        CurrentTest->TestTrue(
          TEXT("returns valid Data object"), IsValid(Output.Character.Data)
        );

        CurrentTest->TestEqual(
          TEXT("returns character last location"),
          Output.Character.Data->GetStringField(TEXT("lastLocation")),
          Data->GetStringField(TEXT("lastLocation"))
        );

        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

bool FMockRealmsCharactersCreate::RunTest(const FString &Parameters) {
  URedwoodClientInterface *Redwood = NewObject<URedwoodClientInterface>();
  UAsyncTestContext *Context = NewObject<UAsyncTestContext>();

  Redwood->AddToRoot();
  Context->AddToRoot();

  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsCharactersCreateInitialize(Redwood, Context, 0)
  );
  ADD_LATENT_AUTOMATION_COMMAND(
    FMockRealmsCharactersCreateRun(Redwood, Context, 1)
  );

  ADD_LATENT_AUTOMATION_COMMAND(FWaitForEnd(Redwood, Context, 2));

  Context->Start();

  return true;
}

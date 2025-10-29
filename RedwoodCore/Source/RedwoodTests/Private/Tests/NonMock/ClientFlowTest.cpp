// Copyright Incanta Games. All Rights Reserved.

#include "../../TestCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FClientFlowTest,
  "Redwood.NonMock.Client.Flow",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
);

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FInitialize);
void FInitialize::Initialize() {
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

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FRegister);
void FRegister::Initialize() {
  Redwood->Register(
    "user1",
    "password",
    FRedwoodAuthUpdateDelegate::CreateLambda(
      [this](const FRedwoodAuthUpdate &Result) {
        if (Result.Message == TEXT("User already registered")) {
          Context->bIsCurrentTestComplete = true;
          return;
        }

        CurrentTest->TestEqual(
          TEXT("Register Message"), Result.Message, TEXT("")
        );
        CurrentTest->TestEqual(
          TEXT("Register Success"), Result.Type, ERedwoodAuthUpdateType::Success
        );
        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FLogin);
void FLogin::Initialize() {
  Redwood->Login(
    "user1",
    "password",
    "local",
    false,
    FRedwoodAuthUpdateDelegate::CreateLambda(
      [this](const FRedwoodAuthUpdate &Result) {
        CurrentTest->TestEqual(
          TEXT("Login Success"), Result.Type, ERedwoodAuthUpdateType::Success
        );
        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FInitializeRealm);
void FInitializeRealm::Initialize() {
  Redwood->ListRealms(FRedwoodListRealmsOutputDelegate::CreateLambda(
    [this](const FRedwoodListRealmsOutput &Output) {
      CurrentTest->TestEqual(
        TEXT("ListRealms no error"), Output.Error, TEXT("")
      );
      CurrentTest->TestEqual(
        TEXT("ListRealms returns 2 realms"), Output.Realms.Num(), 2
      );

      Redwood->InitializeRealmConnection(
        Output.Realms[0],
        FRedwoodSocketConnectedDelegate::CreateLambda(
          [this](const FRedwoodSocketConnected &Result) {
            CurrentTest->TestEqual(
              TEXT("Realm Connection Success"), Result.Error, TEXT("")
            );
            Context->bIsCurrentTestComplete = true;
          }
        )
      );
    }
  ));
}

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FListNoCharacters);
void FListNoCharacters::Initialize() {
  Redwood->ListCharacters(FRedwoodListCharactersOutputDelegate::CreateLambda(
    [this](const FRedwoodListCharactersOutput &Output) {
      CurrentTest->TestEqual(
        TEXT("ListCharacters no error"), Output.Error, TEXT("")
      );
      CurrentTest->TestEqual(
        TEXT("ListCharacters returns no characters"), Output.Characters.Num(), 0
      );
      Context->bIsCurrentTestComplete = true;
    }
  ));
}

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FCreateCharacter);
void FCreateCharacter::Initialize() {
  USIOJsonObject *CharacterCreatorData = NewObject<USIOJsonObject>();

  Redwood->CreateCharacter(
    TEXT("TestCharacter"),
    CharacterCreatorData,
    FRedwoodGetCharacterOutputDelegate::CreateLambda(
      [this](const FRedwoodGetCharacterOutput &Output) {
        CurrentTest->TestEqual(
          TEXT("CreateCharacter no error"), Output.Error, TEXT("")
        );
        CurrentTest->TestTrue(
          TEXT("CreateCharacter has valid character"),
          !Output.Character.PlayerId.IsEmpty()
        );
        CurrentTest->TestEqual(
          TEXT("CreateCharacter has correct name"),
          Output.Character.Name,
          TEXT("TestCharacter")
        );

        Context->Data.SetStringField("CharacterId", Output.Character.Id);

        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FListCharacters);
void FListCharacters::Initialize() {
  Redwood->ListCharacters(FRedwoodListCharactersOutputDelegate::CreateLambda(
    [this](const FRedwoodListCharactersOutput &Output) {
      CurrentTest->TestEqual(
        TEXT("ListCharacters no error"), Output.Error, TEXT("")
      );
      CurrentTest->TestEqual(
        TEXT("ListCharacters returns character"), Output.Characters.Num(), 1
      );
      Context->bIsCurrentTestComplete = true;
    }
  ));
}

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FSetCharacter);
void FSetCharacter::Initialize() {
  Redwood->SetCharacterData(
    Context->Data.GetStringField(TEXT("CharacterId")),
    TEXT("TestCharacter 2"),
    nullptr,
    FRedwoodGetCharacterOutputDelegate::CreateLambda(
      [this](const FRedwoodGetCharacterOutput &Output) {
        CurrentTest->TestEqual(
          TEXT("SetCharacter no error"), Output.Error, TEXT("")
        );
        CurrentTest->TestTrue(
          TEXT("SetCharacter has valid character"),
          !Output.Character.PlayerId.IsEmpty()
        );
        CurrentTest->TestEqual(
          TEXT("SetCharacter has correct name"),
          Output.Character.Name,
          TEXT("TestCharacter 2")
        );
        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FGetCharacter);
void FGetCharacter::Initialize() {
  Redwood->GetCharacterData(
    Context->Data.GetStringField(TEXT("CharacterId")),
    FRedwoodGetCharacterOutputDelegate::CreateLambda(
      [this](const FRedwoodGetCharacterOutput &Output) {
        CurrentTest->TestEqual(
          TEXT("GetCharacter no error"), Output.Error, TEXT("")
        );
        CurrentTest->TestTrue(
          TEXT("GetCharacter has valid character"),
          !Output.Character.PlayerId.IsEmpty()
        );
        CurrentTest->TestEqual(
          TEXT("GetCharacter has correct name"),
          Output.Character.Name,
          TEXT("TestCharacter 2")
        );
        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FListNoServers);
void FListNoServers::Initialize() {
  Redwood->ListProxies(
    TArray<FString>(),
    FRedwoodListProxiesOutputDelegate::CreateLambda(
      [this](const FRedwoodListProxiesOutput &Output) {
        CurrentTest->TestEqual(
          TEXT("ListProxies no error"), Output.Error, TEXT("")
        );
        CurrentTest->TestEqual(
          TEXT("ListProxies returns no servers"), Output.Proxies.Num(), 0
        );
        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FCreatePublicServer);
void FCreatePublicServer::Initialize() {
  FRedwoodCreateProxyInput Parameters;
  Parameters.Name = TEXT("Test Server");
  Parameters.Region = TEXT("local");
  Parameters.ModeId = TEXT("match");
  Parameters.MapId = TEXT("map");
  Parameters.bPublic = true;
  Parameters.bProxyEndsWhenCollectionEnds = true;
  Parameters.bContinuousPlay = false;
  Parameters.Password = TEXT("");
  Parameters.ShortCode = TEXT("");

  Redwood->CreateProxy(
    false,
    Parameters,
    FRedwoodCreateProxyOutputDelegate::CreateLambda(
      [this](const FRedwoodCreateProxyOutput &Output) {
        CurrentTest->TestEqual(
          TEXT("CreateProxy no error"), Output.Error, TEXT("")
        );
        CurrentTest->TestTrue(
          TEXT("CreateProxy has valid server"), !Output.ProxyReference.IsEmpty()
        );
        Context->Data.SetStringField("ProxyReference", Output.ProxyReference);
        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FListProxies);
void FListProxies::Initialize() {
  Redwood->ListProxies(
    TArray<FString>(),
    FRedwoodListProxiesOutputDelegate::CreateLambda(
      [this](const FRedwoodListProxiesOutput &Output) {
        CurrentTest->TestEqual(
          TEXT("ListProxies no error"), Output.Error, TEXT("")
        );
        CurrentTest->TestEqual(
          TEXT("ListProxies returns server"), Output.Proxies.Num(), 1
        );
        Context->bIsCurrentTestComplete = true;
      }
    )
  );
}

DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(FStopProxy);
void FStopProxy::Initialize() {
  Redwood->StopProxy(
    Context->Data.GetStringField(TEXT("ProxyReference")),
    FRedwoodErrorOutputDelegate::CreateLambda([this](const FString &Error) {
      CurrentTest->TestEqual(TEXT("StopProxy no error"), Error, TEXT(""));
      Context->bIsCurrentTestComplete = true;
    })
  );
}

bool FClientFlowTest::RunTest(const FString &Parameters) {
  URedwoodClientInterface *Redwood = NewObject<URedwoodClientInterface>();
  UAsyncTestContext *Context = NewObject<UAsyncTestContext>();

  Redwood->AddToRoot();
  Context->AddToRoot();

  ADD_LATENT_AUTOMATION_COMMAND(FInitialize(Redwood, Context, 0));
  ADD_LATENT_AUTOMATION_COMMAND(FRegister(Redwood, Context, 1));
  ADD_LATENT_AUTOMATION_COMMAND(FLogin(Redwood, Context, 2));
  ADD_LATENT_AUTOMATION_COMMAND(FInitializeRealm(Redwood, Context, 3));
  ADD_LATENT_AUTOMATION_COMMAND(FListNoCharacters(Redwood, Context, 4));
  ADD_LATENT_AUTOMATION_COMMAND(FCreateCharacter(Redwood, Context, 5));
  ADD_LATENT_AUTOMATION_COMMAND(FListCharacters(Redwood, Context, 6));
  ADD_LATENT_AUTOMATION_COMMAND(FSetCharacter(Redwood, Context, 7));
  ADD_LATENT_AUTOMATION_COMMAND(FGetCharacter(Redwood, Context, 8));
  ADD_LATENT_AUTOMATION_COMMAND(FListNoServers(Redwood, Context, 9));
  ADD_LATENT_AUTOMATION_COMMAND(FCreatePublicServer(Redwood, Context, 10));
  ADD_LATENT_AUTOMATION_COMMAND(FListProxies(Redwood, Context, 11));
  ADD_LATENT_AUTOMATION_COMMAND(FStopProxy(Redwood, Context, 12));
  ADD_LATENT_AUTOMATION_COMMAND(FListNoServers(Redwood, Context, 13));

  ADD_LATENT_AUTOMATION_COMMAND(FWaitForEnd(Redwood, Context, 14));

  Context->Start();

  return true;
}

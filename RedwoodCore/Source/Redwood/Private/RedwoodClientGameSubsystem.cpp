// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodClientGameSubsystem.h"
#include "RedwoodClientInterface.h"
#include "RedwoodCommonGameSubsystem.h"
#include "RedwoodGameplayTags.h"
#include "RedwoodSettings.h"

#if WITH_EDITOR
  #include "RedwoodEditorSettings.h"
#endif

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LatencyCheckerLibrary.h"
#include "SocketIOClient.h"

#include "JsonModern.h"

void URedwoodClientGameSubsystem::Initialize(
  FSubsystemCollectionBase &Collection
) {
  Super::Initialize(Collection);

  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface = NewObject<URedwoodClientInterface>();

    ClientInterface->OnPingsReceived.AddDynamic(
      this, &URedwoodClientGameSubsystem::HandlePingsReceived
    );
    ClientInterface->OnRequestToJoinServer.AddDynamic(
      this, &URedwoodClientGameSubsystem::HandleRequestToJoinServer
    );
    ClientInterface->OnDirectorConnectionLost.AddDynamic(
      this, &URedwoodClientGameSubsystem::HandleOnDirectorConnectionLost
    );
    ClientInterface->OnDirectorConnectionReestablished.AddDynamic(
      this,
      &URedwoodClientGameSubsystem::HandleOnDirectorConnectionReestablished
    );
    ClientInterface->OnRealmConnectionLost.AddDynamic(
      this, &URedwoodClientGameSubsystem::HandleOnRealmConnectionLost
    );
  }
}

void URedwoodClientGameSubsystem::Deinitialize() {
  Super::Deinitialize();

  if (ClientInterface) {
    ClientInterface->Deinitialize();
  }
}

void URedwoodClientGameSubsystem::InitializeDirectorConnection(
  FRedwoodSocketConnectedDelegate OnDirectorConnected
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->InitializeDirectorConnection(OnDirectorConnected);
  } else {
    FRedwoodSocketConnected Output;
    OnDirectorConnected.ExecuteIfBound(Output);
  }
}

bool URedwoodClientGameSubsystem::IsDirectorConnected() {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    return ClientInterface->IsDirectorConnected();
  } else {
    return true;
  }
}

void URedwoodClientGameSubsystem::Register(
  const FString &Username,
  const FString &Password,
  FRedwoodAuthUpdateDelegate OnUpdate
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->Register(Username, Password, OnUpdate);
  } else {
    FRedwoodAuthUpdate Output;
    Output.Type = ERedwoodAuthUpdateType::Error;
    Output.Message =
      "Cannot register in when connecting to the backend is disabled in PIE, just login with any credentials";
    OnUpdate.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::Logout() {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->Logout();
  }
}

bool URedwoodClientGameSubsystem::IsLoggedIn() {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    return ClientInterface->IsLoggedIn();
  } else {
    return true;
  }
}

void URedwoodClientGameSubsystem::AttemptAutoLogin(
  FRedwoodAuthUpdateDelegate OnUpdate
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->AttemptAutoLogin(OnUpdate);
  } else {
    FRedwoodAuthUpdate Output;
    Output.Type = ERedwoodAuthUpdateType::Success;
    OnUpdate.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::Login(
  const FString &Username,
  const FString &PasswordOrToken,
  const FString &Provider,
  bool bRememberMe,
  FRedwoodAuthUpdateDelegate OnUpdate
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->Login(
      Username, PasswordOrToken, Provider, bRememberMe, OnUpdate
    );
  } else {
    FRedwoodAuthUpdate Output;
    Output.Type = ERedwoodAuthUpdateType::Success;
    OnUpdate.ExecuteIfBound(Output);
  }
}

FString URedwoodClientGameSubsystem::GetNickname() {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    return ClientInterface->GetNickname();
  } else {
    return TEXT("PIE Player");
  }
}

void URedwoodClientGameSubsystem::CancelWaitingForAccountVerification() {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->CancelWaitingForAccountVerification();
  }
}

void URedwoodClientGameSubsystem::ListRealms(
  FRedwoodListRealmsOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->ListRealms(OnOutput);
  } else {
    TArray<FRedwoodRealm> Realms;

    FRedwoodRealm FakeRealm;
    FakeRealm.Id = FGuid::NewGuid().ToString();
    FakeRealm.Name = "Local PIE Realm";
    FakeRealm.Uri = "ws://localhost";
    FakeRealm.bListed = true;

    FRedwoodListRealmsOutput Output;
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::InitializeConnectionForFirstRealm(
  FRedwoodSocketConnectedDelegate OnRealmConnected
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->InitializeConnectionForFirstRealm(OnRealmConnected);
  } else {
    FRedwoodSocketConnected Output;
    OnRealmConnected.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::InitializeRealmConnection(
  FRedwoodRealm InRealm, FRedwoodSocketConnectedDelegate OnRealmConnected
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->InitializeRealmConnection(InRealm, OnRealmConnected);
  } else {
    FRedwoodSocketConnected Output;
    OnRealmConnected.ExecuteIfBound(Output);
  }
}

bool URedwoodClientGameSubsystem::IsRealmConnected() {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    return ClientInterface->IsRealmConnected();
  } else {
    return true;
  }
}

TMap<FString, float> URedwoodClientGameSubsystem::GetRegions() {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    return ClientInterface->GetRegions();
  } else {
    TMap<FString, float> Regions;
    Regions.Add("Local", 0.0f);
    return Regions;
  }
}

void URedwoodClientGameSubsystem::ListCharacters(
  FRedwoodListCharactersOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->ListCharacters(OnOutput);
  } else {
    TArray<FRedwoodCharacterBackend> Characters;

    FRedwoodListCharactersOutput Output;
    Output.Characters =
      URedwoodCommonGameSubsystem::LoadAllCharactersFromDisk();
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::CreateCharacter(
  FString Name,
  USIOJsonObject *CharacterCreatorData,
  FRedwoodGetCharacterOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->CreateCharacter(Name, CharacterCreatorData, OnOutput);
  } else {
    FRedwoodCharacterBackend Character;
    uint8 CharacterIndex =
      URedwoodCommonGameSubsystem::GetCharactersOnDiskCount();
    FString PaddedCharacterIndex =
      FString::Printf(TEXT("%03d"), CharacterIndex);
    Character.Id = PaddedCharacterIndex;
    Character.PlayerId = FGuid::NewGuid().ToString();
    Character.Name = Name;
    Character.CharacterCreatorData = CharacterCreatorData;
    Character.Metadata = NewObject<USIOJsonObject>();
    Character.EquippedInventory = NewObject<USIOJsonObject>();
    Character.NonequippedInventory = NewObject<USIOJsonObject>();
    Character.Data = NewObject<USIOJsonObject>();

    FRedwoodGetCharacterOutput Output;
    Output.Character = Character;

    URedwoodCommonGameSubsystem::SaveCharacterToDisk(Character);

    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::GetCharacterData(
  FString CharacterId, FRedwoodGetCharacterOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->GetCharacterData(CharacterId, OnOutput);
  } else {
    FRedwoodGetCharacterOutput Output;
    Output.Character =
      URedwoodCommonGameSubsystem::LoadCharacterFromDisk(CharacterId);
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::SetCharacterData(
  FString CharacterId,
  FString Name,
  USIOJsonObject *CharacterCreatorData,
  FRedwoodGetCharacterOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->SetCharacterData(
      CharacterId, Name, CharacterCreatorData, OnOutput
    );
  } else {
    FRedwoodCharacterBackend Character =
      URedwoodCommonGameSubsystem::LoadCharacterFromDisk(CharacterId);

    if (!Name.IsEmpty()) {
      Character.Name = Name;
    }

    if (CharacterCreatorData) {
      Character.CharacterCreatorData = CharacterCreatorData;
    }

    FRedwoodGetCharacterOutput Output;
    Output.Character = Character;

    URedwoodCommonGameSubsystem::SaveCharacterToDisk(Character);

    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::SetSelectedCharacter(FString CharacterId) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->SetSelectedCharacter(CharacterId);
  }
}

void URedwoodClientGameSubsystem::JoinMatchmaking(
  FString ProfileId,
  TArray<FString> InRegions,
  FRedwoodTicketingUpdateDelegate OnUpdate
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->JoinMatchmaking(ProfileId, InRegions, OnUpdate);
  } else {
    FRedwoodTicketingUpdate Output;
    Output.Type = ERedwoodTicketingUpdateType::JoinResponse;
    Output.Message =
      "Cannot join matchmaking in PIE when connecting to the backend is disabled";
    OnUpdate.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::JoinQueue(
  FString ProxyId, FString ZoneName, FRedwoodTicketingUpdateDelegate OnUpdate
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->JoinQueue(ProxyId, ZoneName, OnUpdate);
  } else {
    FRedwoodTicketingUpdate Output;
    Output.Type = ERedwoodTicketingUpdateType::JoinResponse;
    Output.Message =
      "Cannot join matchmaking in PIE when connecting to the backend is disabled";
    OnUpdate.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::LeaveTicketing(
  FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->LeaveTicketing(OnOutput);
  } else {
    FString Error =
      "Cannot leave ticketing in PIE when connecting to the backend is disabled";
    OnOutput.ExecuteIfBound(Error);
  }
}

void URedwoodClientGameSubsystem::ListServers(
  TArray<FString> PrivateServerReferences,
  FRedwoodListServersOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->ListServers(PrivateServerReferences, OnOutput);
  } else {
    FRedwoodListServersOutput Output;
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::CreateServer(
  bool bJoinSession,
  FRedwoodCreateServerInput Parameters,
  FRedwoodCreateServerOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->CreateServer(bJoinSession, Parameters, OnOutput);
  } else {
    FRedwoodCreateServerOutput Output;
    Output.Error =
      "Cannot create server in PIE when connecting to the backend is disabled";
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::JoinServerInstance(
  FString ServerReference,
  FString Password,
  FRedwoodJoinServerOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->JoinServerInstance(ServerReference, Password, OnOutput);
  } else {
    FRedwoodJoinServerOutput Output;
    Output.Error =
      "Cannot join server in PIE when connecting to the backend is disabled";
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::StopServer(
  FString ServerProxyId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    ClientInterface->StopServer(ServerProxyId, OnOutput);
  } else {
    FString Error =
      "Cannot stop server in PIE when connecting to the backend is disabled";
    OnOutput.ExecuteIfBound(Error);
  }
}

FString URedwoodClientGameSubsystem::GetConnectionConsoleCommand() {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend()) {
    return ClientInterface->GetConnectionConsoleCommand();
  } else {
    return FString();
  }
}

void URedwoodClientGameSubsystem::HandlePingsReceived() {
  OnPingsReceived.Broadcast();
}

void URedwoodClientGameSubsystem::HandleRequestToJoinServer(
  FString ConsoleCommand
) {
  URedwoodSettings *RedwoodSettings = GetMutableDefault<URedwoodSettings>();
  if (RedwoodSettings->bAutoConnectToServers) {
    UWorld *World = GetGameInstance()->GetWorld();
    if (IsValid(World)) {
      UKismetSystemLibrary::ExecuteConsoleCommand(World, ConsoleCommand);
    } else {
      UE_LOG(
        LogRedwood, Error, TEXT("World is not valid when trying to join server")
      );
    }
  } else {
    OnRequestToJoinServer.Broadcast(ConsoleCommand);
  }
}

void URedwoodClientGameSubsystem::HandleOnDirectorConnectionLost() {
  OnDirectorConnectionLost.Broadcast();
}

void URedwoodClientGameSubsystem::HandleOnDirectorConnectionReestablished() {
  OnDirectorConnectionReestablished.Broadcast();
}

void URedwoodClientGameSubsystem::HandleOnRealmConnectionLost() {
  OnRealmConnectionLost.Broadcast();
}

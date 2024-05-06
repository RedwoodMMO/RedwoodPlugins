// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodTitleGameSubsystem.h"
#include "RedwoodGameplayTags.h"
#include "RedwoodSettings.h"
#include "RedwoodTitleInterface.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/KismetStringLibrary.h"
#include "LatencyCheckerLibrary.h"
#include "SocketIOClient.h"

#include "JsonModern.h"

void URedwoodTitleGameSubsystem::Initialize(FSubsystemCollectionBase &Collection
) {
  Super::Initialize(Collection);

  TitleInterface = NewObject<URedwoodTitleInterface>();

  TitleInterface->OnPingsReceived.AddDynamic(
    this, &URedwoodTitleGameSubsystem::HandlePingsReceived
  );
  TitleInterface->OnRequestToJoinServer.AddDynamic(
    this, &URedwoodTitleGameSubsystem::HandleRequestToJoinServer
  );
}

void URedwoodTitleGameSubsystem::Deinitialize() {
  Super::Deinitialize();

  TitleInterface->Deinitialize();
}

void URedwoodTitleGameSubsystem::InitializeDirectorConnection(
  FRedwoodSocketConnectedDelegate OnDirectorConnected
) {
  TitleInterface->InitializeDirectorConnection(OnDirectorConnected);
}

bool URedwoodTitleGameSubsystem::IsDirectorConnected() {
  return TitleInterface->IsDirectorConnected();
}

void URedwoodTitleGameSubsystem::Register(
  const FString &Username,
  const FString &Password,
  FRedwoodAuthUpdateDelegate OnUpdate
) {
  TitleInterface->Register(Username, Password, OnUpdate);
}

void URedwoodTitleGameSubsystem::Logout() {
  TitleInterface->Logout();
}

bool URedwoodTitleGameSubsystem::IsLoggedIn() {
  return TitleInterface->IsLoggedIn();
}

void URedwoodTitleGameSubsystem::AttemptAutoLogin(
  FRedwoodAuthUpdateDelegate OnUpdate
) {
  TitleInterface->AttemptAutoLogin(OnUpdate);
}

void URedwoodTitleGameSubsystem::Login(
  const FString &Username,
  const FString &PasswordOrToken,
  bool bRememberMe,
  FRedwoodAuthUpdateDelegate OnUpdate
) {
  TitleInterface->Login(Username, PasswordOrToken, bRememberMe, OnUpdate);
}

void URedwoodTitleGameSubsystem::CancelWaitingForAccountVerification() {
  TitleInterface->CancelWaitingForAccountVerification();
}

void URedwoodTitleGameSubsystem::ListRealms(
  FRedwoodListRealmsOutputDelegate OnOutput
) {
  TitleInterface->ListRealms(OnOutput);
}

void URedwoodTitleGameSubsystem::InitializeConnectionForFirstRealm(
  FRedwoodSocketConnectedDelegate OnRealmConnected
) {
  TitleInterface->InitializeConnectionForFirstRealm(OnRealmConnected);
}

void URedwoodTitleGameSubsystem::InitializeRealmConnection(
  FRedwoodRealm InRealm, FRedwoodSocketConnectedDelegate OnRealmConnected
) {
  TitleInterface->InitializeRealmConnection(InRealm, OnRealmConnected);
}

bool URedwoodTitleGameSubsystem::IsRealmConnected() {
  return TitleInterface->IsRealmConnected();
}

TMap<FString, float> URedwoodTitleGameSubsystem::GetRegions() {
  return TitleInterface->GetRegions();
}

void URedwoodTitleGameSubsystem::ListCharacters(
  FRedwoodListCharactersOutputDelegate OnOutput
) {
  TitleInterface->ListCharacters(OnOutput);
}

void URedwoodTitleGameSubsystem::CreateCharacter(
  USIOJsonObject *Metadata,
  USIOJsonObject *EquippedInventory,
  USIOJsonObject *NonequippedInventory,
  USIOJsonObject *Data,
  FRedwoodGetCharacterOutputDelegate OnOutput
) {
  TitleInterface->CreateCharacter(
    Metadata, EquippedInventory, NonequippedInventory, Data, OnOutput
  );
}

void URedwoodTitleGameSubsystem::GetCharacterData(
  FString CharacterId, FRedwoodGetCharacterOutputDelegate OnOutput
) {
  TitleInterface->GetCharacterData(CharacterId, OnOutput);
}

void URedwoodTitleGameSubsystem::SetCharacterData(
  FString CharacterId,
  USIOJsonObject *Metadata,
  USIOJsonObject *EquippedInventory,
  USIOJsonObject *NonequippedInventory,
  USIOJsonObject *Data,
  FRedwoodGetCharacterOutputDelegate OnOutput
) {
  TitleInterface->SetCharacterData(
    CharacterId,
    Metadata,
    EquippedInventory,
    NonequippedInventory,
    Data,
    OnOutput
  );
}

void URedwoodTitleGameSubsystem::SetSelectedCharacter(FString CharacterId) {
  TitleInterface->SetSelectedCharacter(CharacterId);
}

void URedwoodTitleGameSubsystem::JoinTicketing(
  TArray<FString> ModeIds,
  TArray<FString> InRegions,
  FRedwoodTicketingUpdateDelegate OnUpdate
) {
  TitleInterface->JoinTicketing(ModeIds, InRegions, OnUpdate);
}

void URedwoodTitleGameSubsystem::LeaveTicketing(
  FRedwoodErrorOutputDelegate OnOutput
) {
  TitleInterface->LeaveTicketing(OnOutput);
}

FRedwoodGameServerProxy URedwoodTitleGameSubsystem::ParseServerProxy(
  TSharedPtr<FJsonObject> ServerProxy
) {
  return URedwoodTitleInterface::ParseServerProxy(ServerProxy);
}

FRedwoodGameServerInstance URedwoodTitleGameSubsystem::ParseServerInstance(
  TSharedPtr<FJsonObject> ServerInstance
) {
  return URedwoodTitleInterface::ParseServerInstance(ServerInstance);
}

void URedwoodTitleGameSubsystem::ListServers(
  TArray<FString> PrivateServerReferences,
  FRedwoodListServersOutputDelegate OnOutput
) {
  TitleInterface->ListServers(PrivateServerReferences, OnOutput);
}

void URedwoodTitleGameSubsystem::CreateServer(
  bool bJoinSession,
  FRedwoodCreateServerInput Parameters,
  FRedwoodCreateServerOutputDelegate OnOutput
) {
  TitleInterface->CreateServer(bJoinSession, Parameters, OnOutput);
}

void URedwoodTitleGameSubsystem::GetServerInstance(
  FString ServerReference,
  FString Password,
  bool bJoinSession,
  FRedwoodGetServerOutputDelegate OnOutput
) {
  UE_LOG(
    LogRedwood,
    Log,
    TEXT("GetServerInstance: %s"),
    bJoinSession ? TEXT("true") : TEXT("false")
  );
  TitleInterface->GetServerInstance(
    ServerReference, Password, bJoinSession, OnOutput
  );
}

void URedwoodTitleGameSubsystem::StopServer(
  FString ServerProxyId, FRedwoodErrorOutputDelegate OnOutput
) {
  TitleInterface->StopServer(ServerProxyId, OnOutput);
}

FString URedwoodTitleGameSubsystem::GetConnectionConsoleCommand() {
  return TitleInterface->GetConnectionConsoleCommand();
}

void URedwoodTitleGameSubsystem::HandlePingsReceived() {
  OnPingsReceived.Broadcast();
}

void URedwoodTitleGameSubsystem::HandleRequestToJoinServer(
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

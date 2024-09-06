// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodClientGameSubsystem.h"
#include "RedwoodClientInterface.h"
#include "RedwoodGameplayTags.h"
#include "RedwoodSettings.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/KismetStringLibrary.h"
#include "LatencyCheckerLibrary.h"
#include "SocketIOClient.h"

#include "JsonModern.h"

void URedwoodClientGameSubsystem::Initialize(
  FSubsystemCollectionBase &Collection
) {
  Super::Initialize(Collection);

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
    this, &URedwoodClientGameSubsystem::HandleOnDirectorConnectionReestablished
  );
  ClientInterface->OnRealmConnectionLost.AddDynamic(
    this, &URedwoodClientGameSubsystem::HandleOnRealmConnectionLost
  );
}

void URedwoodClientGameSubsystem::Deinitialize() {
  Super::Deinitialize();

  ClientInterface->Deinitialize();
}

void URedwoodClientGameSubsystem::InitializeDirectorConnection(
  FRedwoodSocketConnectedDelegate OnDirectorConnected
) {
  ClientInterface->InitializeDirectorConnection(OnDirectorConnected);
}

bool URedwoodClientGameSubsystem::IsDirectorConnected() {
  return ClientInterface->IsDirectorConnected();
}

void URedwoodClientGameSubsystem::Register(
  const FString &Username,
  const FString &Password,
  FRedwoodAuthUpdateDelegate OnUpdate
) {
  ClientInterface->Register(Username, Password, OnUpdate);
}

void URedwoodClientGameSubsystem::Logout() {
  ClientInterface->Logout();
}

bool URedwoodClientGameSubsystem::IsLoggedIn() {
  return ClientInterface->IsLoggedIn();
}

void URedwoodClientGameSubsystem::AttemptAutoLogin(
  FRedwoodAuthUpdateDelegate OnUpdate
) {
  ClientInterface->AttemptAutoLogin(OnUpdate);
}

void URedwoodClientGameSubsystem::Login(
  const FString &Username,
  const FString &PasswordOrToken,
  const FString &Provider,
  bool bRememberMe,
  FRedwoodAuthUpdateDelegate OnUpdate
) {
  ClientInterface->Login(
    Username, PasswordOrToken, Provider, bRememberMe, OnUpdate
  );
}

FString URedwoodClientGameSubsystem::GetNickname() {
  return ClientInterface->GetNickname();
}

void URedwoodClientGameSubsystem::CancelWaitingForAccountVerification() {
  ClientInterface->CancelWaitingForAccountVerification();
}

void URedwoodClientGameSubsystem::ListRealms(
  FRedwoodListRealmsOutputDelegate OnOutput
) {
  ClientInterface->ListRealms(OnOutput);
}

void URedwoodClientGameSubsystem::InitializeConnectionForFirstRealm(
  FRedwoodSocketConnectedDelegate OnRealmConnected
) {
  ClientInterface->InitializeConnectionForFirstRealm(OnRealmConnected);
}

void URedwoodClientGameSubsystem::InitializeRealmConnection(
  FRedwoodRealm InRealm, FRedwoodSocketConnectedDelegate OnRealmConnected
) {
  ClientInterface->InitializeRealmConnection(InRealm, OnRealmConnected);
}

bool URedwoodClientGameSubsystem::IsRealmConnected() {
  return ClientInterface->IsRealmConnected();
}

TMap<FString, float> URedwoodClientGameSubsystem::GetRegions() {
  return ClientInterface->GetRegions();
}

void URedwoodClientGameSubsystem::ListCharacters(
  FRedwoodListCharactersOutputDelegate OnOutput
) {
  ClientInterface->ListCharacters(OnOutput);
}

void URedwoodClientGameSubsystem::CreateCharacter(
  FString Name,
  USIOJsonObject *Metadata,
  USIOJsonObject *EquippedInventory,
  USIOJsonObject *NonequippedInventory,
  USIOJsonObject *Data,
  FRedwoodGetCharacterOutputDelegate OnOutput
) {
  ClientInterface->CreateCharacter(
    Name, Metadata, EquippedInventory, NonequippedInventory, Data, OnOutput
  );
}

void URedwoodClientGameSubsystem::GetCharacterData(
  FString CharacterId, FRedwoodGetCharacterOutputDelegate OnOutput
) {
  ClientInterface->GetCharacterData(CharacterId, OnOutput);
}

void URedwoodClientGameSubsystem::SetCharacterData(
  FString CharacterId,
  FString Name,
  USIOJsonObject *Metadata,
  USIOJsonObject *EquippedInventory,
  USIOJsonObject *NonequippedInventory,
  USIOJsonObject *Data,
  FRedwoodGetCharacterOutputDelegate OnOutput
) {
  ClientInterface->SetCharacterData(
    CharacterId,
    Name,
    Metadata,
    EquippedInventory,
    NonequippedInventory,
    Data,
    OnOutput
  );
}

void URedwoodClientGameSubsystem::SetSelectedCharacter(FString CharacterId) {
  ClientInterface->SetSelectedCharacter(CharacterId);
}

void URedwoodClientGameSubsystem::JoinMatchmaking(
  FString ProfileId,
  TArray<FString> InRegions,
  FRedwoodTicketingUpdateDelegate OnUpdate
) {
  ClientInterface->JoinMatchmaking(ProfileId, InRegions, OnUpdate);
}

void URedwoodClientGameSubsystem::JoinQueue(
  FString ProxyId, FString ZoneName, FRedwoodTicketingUpdateDelegate OnUpdate
) {
  ClientInterface->JoinQueue(ProxyId, ZoneName, OnUpdate);
}

void URedwoodClientGameSubsystem::LeaveTicketing(
  FRedwoodErrorOutputDelegate OnOutput
) {
  ClientInterface->LeaveTicketing(OnOutput);
}

FRedwoodGameServerProxy URedwoodClientGameSubsystem::ParseServerProxy(
  TSharedPtr<FJsonObject> ServerProxy
) {
  return URedwoodClientInterface::ParseServerProxy(ServerProxy);
}

FRedwoodGameServerInstance URedwoodClientGameSubsystem::ParseServerInstance(
  TSharedPtr<FJsonObject> ServerInstance
) {
  return URedwoodClientInterface::ParseServerInstance(ServerInstance);
}

void URedwoodClientGameSubsystem::ListServers(
  TArray<FString> PrivateServerReferences,
  FRedwoodListServersOutputDelegate OnOutput
) {
  ClientInterface->ListServers(PrivateServerReferences, OnOutput);
}

void URedwoodClientGameSubsystem::CreateServer(
  bool bJoinSession,
  FRedwoodCreateServerInput Parameters,
  FRedwoodCreateServerOutputDelegate OnOutput
) {
  ClientInterface->CreateServer(bJoinSession, Parameters, OnOutput);
}

void URedwoodClientGameSubsystem::JoinServerInstance(
  FString ServerReference,
  FString Password,
  FRedwoodJoinServerOutputDelegate OnOutput
) {
  ClientInterface->JoinServerInstance(ServerReference, Password, OnOutput);
}

void URedwoodClientGameSubsystem::StopServer(
  FString ServerProxyId, FRedwoodErrorOutputDelegate OnOutput
) {
  ClientInterface->StopServer(ServerProxyId, OnOutput);
}

FString URedwoodClientGameSubsystem::GetConnectionConsoleCommand() {
  return ClientInterface->GetConnectionConsoleCommand();
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

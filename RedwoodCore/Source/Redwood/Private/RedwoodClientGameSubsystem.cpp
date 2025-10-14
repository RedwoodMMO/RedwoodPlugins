// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodClientGameSubsystem.h"
#include "RedwoodClientInterface.h"
#include "RedwoodCommonGameSubsystem.h"
#include "RedwoodGameStateComponent.h"
#include "RedwoodGameplayTags.h"
#include "RedwoodSettings.h"

#if WITH_EDITOR
  #include "RedwoodEditorSettings.h"
#endif

#include "GameFramework/GameStateBase.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LatencyCheckerLibrary.h"
#include "SocketIOClient.h"

void URedwoodClientGameSubsystem::Initialize(
  FSubsystemCollectionBase &Collection
) {
  Super::Initialize(Collection);

  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
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
    ClientInterface->OnPartyInvited.AddDynamic(
      this, &URedwoodClientGameSubsystem::HandleOnPartyInvited
    );
    ClientInterface->OnPartyUpdated.AddDynamic(
      this, &URedwoodClientGameSubsystem::HandleOnPartyUpdated
    );
    ClientInterface->OnPartyKicked.AddDynamic(
      this, &URedwoodClientGameSubsystem::HandleOnPartyKicked
    );
    ClientInterface->OnPartyEmoteReceived.AddDynamic(
      this, &URedwoodClientGameSubsystem::HandleOnPartyEmoteReceived
    );
  }

  FWorldDelegates::OnPostWorldInitialization.AddUObject(
    this, &URedwoodClientGameSubsystem::HandleOnWorldAdded
  );
}

void URedwoodClientGameSubsystem::Deinitialize() {
  Super::Deinitialize();

  if (ClientInterface) {
    ClientInterface->Deinitialize();
  }
}

void URedwoodClientGameSubsystem::HandleOnWorldAdded(
  UWorld *World, FWorldInitializationValues IVS
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld()) && IsDirectorConnected()) {
    if (IsValid(World) && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE)) {
      World->GetOnBeginPlayEvent().AddUObject(
        this, &URedwoodClientGameSubsystem::HandleOnWorldBeginPlay
      );
    }
  }
}

void URedwoodClientGameSubsystem::HandleOnWorldBeginPlay(bool bBegunPlay) {
  UWorld *World = GetWorld();

  if (IsValid(World) && bBegunPlay) {
    // This function was using for an older method to report online status
    // but this is now reported by the server instead of the client. We're
    // leaving this function here just in case we need it in the future.
  }
}

void URedwoodClientGameSubsystem::InitializeDirectorConnection(
  FRedwoodSocketConnectedDelegate OnDirectorConnected
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->InitializeDirectorConnection(OnDirectorConnected);
  } else {
    FRedwoodSocketConnected Output;
    OnDirectorConnected.ExecuteIfBound(Output);
  }
}

bool URedwoodClientGameSubsystem::IsDirectorConnected() {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
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
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
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
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->Logout();
  }
}

bool URedwoodClientGameSubsystem::IsLoggedIn() {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    return ClientInterface->IsLoggedIn();
  } else {
    return true;
  }
}

void URedwoodClientGameSubsystem::AttemptAutoLogin(
  FRedwoodAuthUpdateDelegate OnUpdate
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
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
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
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
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    return ClientInterface->GetNickname();
  } else {
    return TEXT("PIE Player");
  }
}

void URedwoodClientGameSubsystem::CancelWaitingForAccountVerification() {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->CancelWaitingForAccountVerification();
  }
}

FString URedwoodClientGameSubsystem::GetPlayerId() {
  if (ClientInterface) {
    return ClientInterface->GetPlayerId();
  }
  return FString();
}

void URedwoodClientGameSubsystem::SearchForPlayers(
  FString UsernameOrNickname,
  bool bIncludePartialMatches,
  FRedwoodListPlayersOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->SearchForPlayers(
      UsernameOrNickname, bIncludePartialMatches, OnOutput
    );
  } else {
    FRedwoodListPlayersOutput Output;
    Output.Error = TEXT("Cannot search for players without using a backend");
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::SearchForPlayerById(
  FString TargetPlayerId, FRedwoodPlayerOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->SearchForPlayerById(TargetPlayerId, OnOutput);
  } else {
    FRedwoodPlayerOutput Output;
    Output.Error = TEXT("Cannot search for players without using a backend");
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::ListFriends(
  ERedwoodFriendListType Filter, FRedwoodListPlayersOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->ListFriends(Filter, OnOutput);
  } else {
    FRedwoodListPlayersOutput Output;
    Output.Error = TEXT("Cannot list friends without using a backend");
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::RequestFriend(
  FString OtherPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->RequestFriend(OtherPlayerId, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(TEXT("Cannot request friend without using a backend"
    ));
  }
}

void URedwoodClientGameSubsystem::RemoveFriend(
  FString OtherPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->RemoveFriend(OtherPlayerId, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(TEXT("Cannot remove friend without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::RespondToFriendRequest(
  FString OtherPlayerId, bool bAccept, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->RespondToFriendRequest(OtherPlayerId, bAccept, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(
      TEXT("Cannot respond to friend request without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::SetPlayerBlocked(
  FString OtherPlayerId, bool bBlocked, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->SetPlayerBlocked(OtherPlayerId, bBlocked, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(TEXT("Cannot block players without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::ListGuilds(
  bool bOnlyPlayersGuilds, FRedwoodListGuildsOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->ListGuilds(bOnlyPlayersGuilds, OnOutput);
  } else {
    FRedwoodListGuildsOutput Output;
    Output.Error = TEXT("Cannot list guilds without using a backend");
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::SearchForGuilds(
  FString SearchText,
  bool bIncludePartialMatches,
  FRedwoodListGuildsOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->SearchForGuilds(
      SearchText, bIncludePartialMatches, OnOutput
    );
  } else {
    FRedwoodListGuildsOutput Output;
    Output.Error = TEXT("Cannot search for guilds without using a backend");
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::GetGuild(
  FString GuildId, FRedwoodGetGuildOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->GetGuild(GuildId, OnOutput);
  } else {
    FRedwoodGetGuildOutput Output;
    Output.Error = TEXT("Cannot get guild without using a backend");
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::GetSelectedGuild(
  FRedwoodGetGuildOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->GetSelectedGuild(OnOutput);
  } else {
    FRedwoodGetGuildOutput Output;
    Output.Error = TEXT("Cannot get selected guild without using a backend");
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::SetSelectedGuild(
  FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->SetSelectedGuild(GuildId, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(
      TEXT("Cannot set selected guild without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::JoinGuild(
  FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->JoinGuild(GuildId, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(TEXT("Cannot join guild without using a backend"));
  }
}

void URedwoodClientGameSubsystem::InviteToGuild(
  FString GuildId, FString TargetPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->InviteToGuild(GuildId, TargetPlayerId, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(
      TEXT("Cannot invite to guild without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::LeaveGuild(
  FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->LeaveGuild(GuildId, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(TEXT("Cannot leave guild without using a backend"));
  }
}

void URedwoodClientGameSubsystem::ListGuildMembers(
  FString GuildId,
  ERedwoodGuildAndAllianceMemberState State,
  FRedwoodListGuildMembersOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->ListGuildMembers(GuildId, State, OnOutput);
  } else {
    FRedwoodListGuildMembersOutput Output;
    Output.Error = TEXT("Cannot list guild members without using a backend");
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::CreateGuild(
  FString GuildName,
  FString GuildTag,
  ERedwoodGuildInviteType InviteType,
  bool bListed,
  bool bMembershipPublic,
  FRedwoodCreateGuildOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->CreateGuild(
      GuildName, GuildTag, InviteType, bListed, bMembershipPublic, OnOutput
    );
  } else {
    FRedwoodCreateGuildOutput Output;
    Output.Error = TEXT("Cannot create guild without using a backend");
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::UpdateGuild(
  FString GuildId,
  FString GuildName,
  FString GuildTag,
  ERedwoodGuildInviteType InviteType,
  bool bListed,
  bool bMembershipPublic,
  FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->UpdateGuild(
      GuildId,
      GuildName,
      GuildTag,
      InviteType,
      bListed,
      bMembershipPublic,
      OnOutput
    );
  } else {
    OnOutput.ExecuteIfBound(TEXT("Cannot update guild without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::KickPlayerFromGuild(
  FString GuildId, FString TargetPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->KickPlayerFromGuild(GuildId, TargetPlayerId, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(
      TEXT("Cannot kick player from guild without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::BanPlayerFromGuild(
  FString GuildId, FString TargetPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->BanPlayerFromGuild(GuildId, TargetPlayerId, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(
      TEXT("Cannot ban player from guild without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::UnbanPlayerFromGuild(
  FString GuildId, FString TargetPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->UnbanPlayerFromGuild(GuildId, TargetPlayerId, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(
      TEXT("Cannot unban player from guild without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::PromotePlayerToGuildAdmin(
  FString GuildId, FString TargetPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->PromotePlayerToGuildAdmin(
      GuildId, TargetPlayerId, OnOutput
    );
  } else {
    OnOutput.ExecuteIfBound(
      TEXT("Cannot promote player to guild admin without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::DemotePlayerFromGuildAdmin(
  FString GuildId, FString TargetPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->DemotePlayerFromGuildAdmin(
      GuildId, TargetPlayerId, OnOutput
    );
  } else {
    OnOutput.ExecuteIfBound(
      TEXT("Cannot demote player from guild admin without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::ListAlliances(
  FString GuildIdFilter, FRedwoodListAlliancesOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->ListAlliances(GuildIdFilter, OnOutput);
  } else {
    FRedwoodListAlliancesOutput Output;
    Output.Error = TEXT("Cannot list alliances without using a backend");
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::SearchForAlliances(
  FString SearchText,
  bool bIncludePartialMatches,
  FRedwoodListAlliancesOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->SearchForAlliances(
      SearchText, bIncludePartialMatches, OnOutput
    );
  } else {
    FRedwoodListAlliancesOutput Output;
    Output.Error = TEXT("Cannot search for alliances without using a backend");
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::CanAdminAlliance(
  FString AllianceId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->CanAdminAlliance(AllianceId, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(
      TEXT("Cannot check alliance admin privileges without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::CreateAlliance(
  FString AllianceName,
  FString GuildId,
  bool bInviteOnly,
  FRedwoodCreateAllianceOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->CreateAlliance(
      AllianceName, GuildId, bInviteOnly, OnOutput
    );
  } else {
    FRedwoodCreateAllianceOutput Output;
    Output.Error = TEXT("Cannot create alliance without using a backend");
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::UpdateAlliance(
  FString AllianceId,
  FString AllianceName,
  bool bInviteOnly,
  FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->UpdateAlliance(
      AllianceId, AllianceName, bInviteOnly, OnOutput
    );
  } else {
    OnOutput.ExecuteIfBound(
      TEXT("Cannot update alliance without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::KickGuildFromAlliance(
  FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->KickGuildFromAlliance(AllianceId, GuildId, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(
      TEXT("Cannot kick guild from alliance without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::BanGuildFromAlliance(
  FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->BanGuildFromAlliance(AllianceId, GuildId, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(
      TEXT("Cannot ban guild from alliance without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::UnbanGuildFromAlliance(
  FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->UnbanGuildFromAlliance(AllianceId, GuildId, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(
      TEXT("Cannot unban guild from alliance without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::ListAllianceGuilds(
  FString AllianceId,
  ERedwoodGuildAndAllianceMemberState State,
  FRedwoodListAllianceGuildsOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->ListAllianceGuilds(AllianceId, State, OnOutput);
  } else {
    FRedwoodListAllianceGuildsOutput Output;
    Output.Error = TEXT("Cannot list alliance guilds without using a backend");
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::JoinAlliance(
  FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->JoinAlliance(AllianceId, GuildId, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(TEXT("Cannot join alliance without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::LeaveAlliance(
  FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->LeaveAlliance(AllianceId, GuildId, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(TEXT("Cannot leave alliance without using a backend"
    ));
  }
}

void URedwoodClientGameSubsystem::InviteGuildToAlliance(
  FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->InviteGuildToAlliance(AllianceId, GuildId, OnOutput);
  } else {
    OnOutput.ExecuteIfBound(
      TEXT("Cannot invite guild to alliance without using a backend")
    );
  }
}

void URedwoodClientGameSubsystem::ListRealms(
  FRedwoodListRealmsOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->ListRealms(OnOutput);
  } else {
    TArray<FRedwoodRealm> Realms;

    FRedwoodRealm FakeRealm;
    FakeRealm.Id = FGuid::NewGuid().ToString();
    FakeRealm.Name = "Local PIE Realm";
    FakeRealm.Uri = "ws://localhost";
    FakeRealm.bListed = true;

    Realms.Add(FakeRealm);

    FRedwoodListRealmsOutput Output;
    Output.Realms = Realms;
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::InitializeConnectionForFirstRealm(
  FRedwoodSocketConnectedDelegate OnRealmConnected
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->InitializeConnectionForFirstRealm(OnRealmConnected);
  } else {
    FRedwoodSocketConnected Output;
    OnRealmConnected.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::InitializeRealmConnection(
  FRedwoodRealm InRealm, FRedwoodSocketConnectedDelegate OnRealmConnected
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->InitializeRealmConnection(InRealm, OnRealmConnected);
  } else {
    FRedwoodSocketConnected Output;
    OnRealmConnected.ExecuteIfBound(Output);
  }
}

bool URedwoodClientGameSubsystem::IsRealmConnected() {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    return ClientInterface->IsRealmConnected();
  } else {
    return true;
  }
}

TMap<FString, float> URedwoodClientGameSubsystem::GetRegions() {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
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
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->ListCharacters(OnOutput);
  } else {
    FRedwoodListCharactersOutput Output;
    Output.Characters =
      URedwoodCommonGameSubsystem::LoadAllCharactersFromDisk();
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::ListArchivedCharacters(
  FRedwoodListCharactersOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->ListArchivedCharacters(OnOutput);
  } else {
    TArray<FRedwoodCharacterBackend> Characters =
      URedwoodCommonGameSubsystem::LoadAllCharactersFromDisk();

    FRedwoodListCharactersOutput Output;

    for (FRedwoodCharacterBackend Character : Characters) {
      if (Character.bArchived) {
        Output.Characters.Add(Character);
      }
    }

    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::CreateCharacter(
  FString Name,
  USIOJsonObject *CharacterCreatorData,
  FRedwoodGetCharacterOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
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
    Character.Progress = NewObject<USIOJsonObject>();
    Character.Data = NewObject<USIOJsonObject>();
    Character.AbilitySystem = NewObject<USIOJsonObject>();

    FRedwoodGetCharacterOutput Output;
    Output.Character = Character;

    URedwoodCommonGameSubsystem::SaveCharacterToDisk(Character);

    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::SetCharacterArchived(
  FString CharacterId, bool bArchived, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->SetCharacterArchived(CharacterId, bArchived, OnOutput);
  } else {
    TSharedPtr<FJsonObject> CharacterObject =
      URedwoodCommonGameSubsystem::LoadCharacterJsonFromDisk(CharacterId);

    if (bArchived) {
      CharacterObject->SetStringField(
        TEXT("archivedAt"), FDateTime::UtcNow().ToString()
      );
    } else {
      TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull);
      CharacterObject->SetField(TEXT("archivedAt"), NullValue);
    }

    URedwoodCommonGameSubsystem::SaveCharacterJsonToDisk(CharacterObject);

    OnOutput.ExecuteIfBound(TEXT(""));
  }
}

void URedwoodClientGameSubsystem::GetCharacterData(
  FString CharacterId, FRedwoodGetCharacterOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
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
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->SetCharacterData(
      CharacterId, Name, CharacterCreatorData, OnOutput
    );
  } else {
    TSharedPtr<FJsonObject> CharacterObject =
      URedwoodCommonGameSubsystem::LoadCharacterJsonFromDisk(CharacterId);

    if (!Name.IsEmpty()) {
      CharacterObject->SetStringField(TEXT("name"), Name);
    }

    if (CharacterCreatorData) {
      CharacterObject->SetObjectField(
        TEXT("characterCreatorData"), CharacterCreatorData->GetRootObject()
      );
    }

    FRedwoodGetCharacterOutput Output;
    Output.Character =
      URedwoodCommonGameSubsystem::ParseCharacter(CharacterObject);

    URedwoodCommonGameSubsystem::SaveCharacterJsonToDisk(CharacterObject);

    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::SetSelectedCharacter(FString CharacterId) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->SetSelectedCharacter(CharacterId);
  }
}

void URedwoodClientGameSubsystem::JoinMatchmaking(
  FString ProfileId,
  TArray<FString> InRegions,
  FRedwoodTicketingUpdateDelegate OnUpdate
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
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
  FString ProxyId,
  FString ZoneName,
  bool bTransferWholeParty,
  FRedwoodTicketingUpdateDelegate OnUpdate
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->JoinQueue(
      ProxyId, ZoneName, bTransferWholeParty, OnUpdate
    );
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
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->LeaveTicketing(OnOutput);
  } else {
    FString Error =
      "Cannot leave ticketing in PIE when connecting to the backend is disabled";
    OnOutput.ExecuteIfBound(Error);
  }
}

void URedwoodClientGameSubsystem::ListProxies(
  TArray<FString> PrivateProxyReferences,
  FRedwoodListProxiesOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->ListProxies(PrivateProxyReferences, OnOutput);
  } else {
    FRedwoodListProxiesOutput Output;
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::CreateProxy(
  bool bJoinSession,
  FRedwoodCreateProxyInput Parameters,
  FRedwoodCreateProxyOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->CreateProxy(bJoinSession, Parameters, OnOutput);
  } else {
    FRedwoodCreateProxyOutput Output;
    Output.Error =
      "Cannot create server in PIE when connecting to the backend is disabled";
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::JoinProxyWithSingleInstance(
  FString ProxyReference,
  FString Password,
  FRedwoodJoinServerOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->JoinProxyWithSingleInstance(
      ProxyReference, Password, OnOutput
    );
  } else {
    FRedwoodJoinServerOutput Output;
    Output.Error =
      "Cannot join server in PIE when connecting to the backend is disabled";
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::StopProxy(
  FString ServerProxyId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->StopProxy(ServerProxyId, OnOutput);
  } else {
    FString Error =
      "Cannot stop server in PIE when connecting to the backend is disabled";
    OnOutput.ExecuteIfBound(Error);
  }
}

FRedwoodParty URedwoodClientGameSubsystem::GetCachedParty() {
  if (ClientInterface) {
    return ClientInterface->GetCachedParty();
  }
  return FRedwoodParty();
}

void URedwoodClientGameSubsystem::GetOrCreateParty(
  bool bCreateIfNotInParty, FRedwoodGetPartyOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->GetOrCreateParty(bCreateIfNotInParty, OnOutput);
  } else {
    FRedwoodGetPartyOutput Output;
    Output.Error =
      "Cannot get or create party in PIE when connecting to the backend is disabled";
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::LeaveParty(
  FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->LeaveParty(OnOutput);
  } else {
    FString Error =
      "Cannot leave party in PIE when connecting to the backend is disabled";
    OnOutput.ExecuteIfBound(Error);
  }
}

void URedwoodClientGameSubsystem::InviteToParty(
  FString TargetPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->InviteToParty(TargetPlayerId, OnOutput);
  } else {
    FString Error =
      "Cannot invite to party in PIE when connecting to the backend is disabled";
    OnOutput.ExecuteIfBound(Error);
  }
}

void URedwoodClientGameSubsystem::ListPartyInvites(
  FRedwoodListPartyInvitesOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->ListPartyInvites(OnOutput);
  } else {
    FRedwoodListPartyInvitesOutput Output;
    Output.Error =
      "Cannot list party invites in PIE when connecting to the backend is disabled";
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::RespondToPartyInvite(
  FString PartyId, bool bAccept, FRedwoodGetPartyOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->RespondToPartyInvite(PartyId, bAccept, OnOutput);
  } else {
    FRedwoodGetPartyOutput Output;
    Output.Error =
      "Cannot respond to party invite in PIE when connecting to the backend is disabled";
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::PromoteToPartyLeader(
  FString TargetPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->PromoteToPartyLeader(TargetPlayerId, OnOutput);
  } else {
    FString Error =
      "Cannot promote to party leader in PIE when connecting to the backend is disabled";
    OnOutput.ExecuteIfBound(Error);
  }
}

void URedwoodClientGameSubsystem::KickFromParty(
  FString TargetPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->KickFromParty(TargetPlayerId, OnOutput);
  } else {
    FString Error =
      "Cannot kick from party in PIE when connecting to the backend is disabled";
    OnOutput.ExecuteIfBound(Error);
  }
}

void URedwoodClientGameSubsystem::SetPartyData(
  FString LootType,
  USIOJsonObject *PartyData,
  FRedwoodGetPartyOutputDelegate OnOutput
) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->SetPartyData(LootType, PartyData, OnOutput);
  } else {
    FRedwoodGetPartyOutput Output;
    Output.Error =
      "Cannot set party data in PIE when connecting to the backend is disabled";
    OnOutput.ExecuteIfBound(Output);
  }
}

void URedwoodClientGameSubsystem::SendEmoteToParty(FString Emote) {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
    ClientInterface->SendEmoteToParty(Emote);
  } else {
    UE_LOG(
      LogRedwood,
      Warning,
      TEXT(
        "Cannot send emote to party in PIE when connecting to the backend is disabled"
      )
    );
  }
}

FString URedwoodClientGameSubsystem::GetConnectionConsoleCommand() {
  if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld())) {
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

void URedwoodClientGameSubsystem::HandleOnPartyInvited(
  FRedwoodPartyInvite Invite
) {
  OnPartyInvited.Broadcast(Invite);
}

void URedwoodClientGameSubsystem::HandleOnPartyUpdated(FRedwoodParty Party) {
  OnPartyUpdated.Broadcast(Party);
}

void URedwoodClientGameSubsystem::HandleOnPartyKicked() {
  OnPartyKicked.Broadcast();
}

void URedwoodClientGameSubsystem::HandleOnPartyEmoteReceived(
  const FString &PlayerId, const FString &Emote
) {
  OnPartyEmoteReceived.Broadcast(PlayerId, Emote);
}

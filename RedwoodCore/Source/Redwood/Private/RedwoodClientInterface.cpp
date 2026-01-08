// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodClientInterface.h"
#include "RedwoodClientGameSubsystem.h"
#include "RedwoodCommonGameSubsystem.h"
#include "RedwoodGameplayTags.h"
#include "RedwoodSaveGame.h"
#include "RedwoodSettings.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"
#include "LatencyCheckerLibrary.h"
#include "Misc/DateTime.h"
#include "SocketIOClient.h"
#include "TimerManager.h"

void URedwoodClientInterface::Deinitialize() {
  if (Director.IsValid()) {
    Director->ClearAllCallbacks();
    Director->Disconnect();
    ISocketIOClientModule::Get().ReleaseNativePointer(Director);
    Director = nullptr;
  }

  if (Realm.IsValid()) {
    Realm->ClearAllCallbacks();
    Realm->Disconnect();
    ISocketIOClientModule::Get().ReleaseNativePointer(Realm);
    Realm = nullptr;
  }
}

void URedwoodClientInterface::Tick(float DeltaTime) {
  TimerManager.Tick(DeltaTime);
}

TStatId URedwoodClientInterface::GetStatId() const {
  RETURN_QUICK_DECLARE_CYCLE_STAT(URedwoodClientInterface, STATGROUP_Tickables);
}

void URedwoodClientInterface::InitializeDirectorConnection(
  FRedwoodSocketConnectedDelegate OnDirectorConnected
) {
  Director = ISocketIOClientModule::Get().NewValidNativePointer();
  bSentDirectorConnected = false;

  Director->OnEvent(
    TEXT("player:account-verified"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      TSharedPtr<FJsonObject> MessageObject = Message->AsObject();
      FString InPlayerId = MessageObject->GetStringField(TEXT("playerId"));

      if (InPlayerId == PlayerId) {
        FRedwoodAuthUpdate Update;
        Update.Type = ERedwoodAuthUpdateType::Success;
        Update.Message = TEXT("");
        OnAccountVerified.ExecuteIfBound(Update);
      }
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  FString Uri = *URedwoodSettings::GetDirectorUri();

  Director->OnReconnectionCallback = [Uri, this](
                                       unsigned ReconnectionAttempt,
                                       unsigned AttemptDelay
                                     ) {
    if (!bSentDirectorConnected && !bSentInitialDirectorConnectionFailureLog) {
      bSentInitialDirectorConnectionFailureLog = true;
      UE_LOG(
        LogRedwood,
        Error,
        TEXT(
          "Unable to establish initial connection to Director at %s; will continue to try to establish connection. See SocketIO plugin logs for retry attempts."
        ),
        *Uri
      );
    } else if (!bDirectorDisconnected) {
      bDirectorDisconnected = true;
      bAuthenticated = false;
      UE_LOG(
        LogRedwood,
        Error,
        TEXT(
          "Lost connection to Director at %s; will continue to try to reestablish connection. See SocketIO plugin logs for retry attempts."
        ),
        *Uri
      );
      OnDirectorConnectionLost.Broadcast();
    }
  };

  Director->OnConnectedCallback = [Uri, OnDirectorConnected, this](
                                    const FString &InSocketId,
                                    const FString &InSessionId
                                  ) {
    bDirectorDisconnected = false;

    if (!bSentDirectorConnected) {
      bSentDirectorConnected = true;
      FRedwoodSocketConnected Details;
      Details.Error = TEXT("");
      OnDirectorConnected.ExecuteIfBound(Details);
      UE_LOG(LogRedwood, Log, TEXT("Connected to Director at %s"), *Uri);
    } else {
      UE_LOG(
        LogRedwood,
        Log,
        TEXT(
          "Reestablished connection to Director at %s, attempting to reauthenticate."
        ),
        *Uri
      );

      Login(
        PlayerId,
        AuthToken,
        "local",
        true,
        FRedwoodAuthUpdateDelegate::CreateLambda([this, OnDirectorConnected](
                                                   const FRedwoodAuthUpdate
                                                     &Update
                                                 ) {
          if (Update.Type == ERedwoodAuthUpdateType::Success) {
            UE_LOG(
              LogRedwood,
              Log,
              TEXT(
                "Reauthenticated connection with Director, calling connection reestablished."
              )
            );
            OnDirectorConnectionReestablished.Broadcast();
          } else {
            UE_LOG(
              LogRedwood,
              Error,
              TEXT("Could not reauthenticate connection with Director: %s"),
              *Update.Message
            );
          }
        }),
        true
      );
    }
  };

  UE_LOG(LogRedwood, Log, TEXT("Connecting to Director at %s"), *Uri);

  Director->Connect(*Uri);
}

bool URedwoodClientInterface::IsDirectorConnected() {
  return Director.IsValid() && Director->bIsConnected;
}

void URedwoodClientInterface::HandleRegionsChanged(
  const FString &Event, const TSharedPtr<FJsonValue> &Message
) {
  FRedwoodRegionsChanged MessageStruct;
  USIOJConvert::JsonObjectToUStruct(
    Message->AsObject(),
    FRedwoodRegionsChanged::StaticStruct(),
    &MessageStruct,
    0,
    0
  );

  Regions.Empty();

  for (FRedwoodRegion Region : MessageStruct.Regions) {
    TSharedPtr<FRedwoodRegionLatency> RegionLatency =
      MakeShareable(new FRedwoodRegionLatency);
    RegionLatency->Id = Region.Name;
    RegionLatency->Url = Region.Ping;
    Regions.Add(RegionLatency->Id, RegionLatency);
  }

  TimerManager.ClearTimer(PingTimer);

  URedwoodSettings *RedwoodSettings = GetMutableDefault<URedwoodSettings>();

  PingAttemptsLeft = RedwoodSettings->PingAttempts;

  InitiatePings();
}

void URedwoodClientInterface::InitiatePings() {
  ULatencyCheckerLibrary::FPingResult Delegate;
  Delegate.BindUFunction(this, FName(TEXT("HandlePingResult")));

  URedwoodSettings *RedwoodSettings = GetMutableDefault<URedwoodSettings>();

  int PingAttempts = RedwoodSettings->PingAttempts;

  if (PingAttemptsLeft > 0) {
    for (auto Itr : Regions) {
      if (Itr.Value->Url.StartsWith("ws") && PingAttemptsLeft == PingAttempts) {
        // websockets only need to iterate once as they'll execute
        // multiple ping attempts internally
        PingAttemptsLeft = 1;
      }

      if (Itr.Value->Url.StartsWith("ws") || PingAttemptsLeft == PingAttempts) {
        // clear the last values
        Itr.Value->RTTs.Empty(PingAttempts);
      }

      if (Itr.Value->Url.StartsWith("ws")) {
        ULatencyCheckerLibrary::PingWebSockets(
          Itr.Value->Url, RedwoodSettings->PingTimeout, PingAttempts, Delegate
        );
      } else {
        ULatencyCheckerLibrary::PingIcmp(
          Itr.Value->Url, RedwoodSettings->PingTimeout, Delegate
        );
      }
    }

    PingAttemptsLeft--;
  } else {
    // we're done pinging, let's store the averages
    bool bHasWebsocketRegion = false;
    for (auto Itr : Regions) {
      if (Itr.Value->Url.StartsWith("ws")) {
        bHasWebsocketRegion = true;
      }

      float Minimum = -1;
      for (float RTT : Itr.Value->RTTs) {
        if (Minimum == -1 || RTT < Minimum) {
          Minimum = RTT;
        }
      }

      if (Minimum >= 0) {
        PingAverages.Add(Itr.Key, Minimum);
      }
    }

    OnPingsReceived.Broadcast();

    // queue the next set of pings
    PingAttemptsLeft = bHasWebsocketRegion ? 1 : PingAttempts;

    TimerManager.SetTimer(
      PingTimer,
      this,
      &URedwoodClientInterface::InitiatePings,
      RedwoodSettings->PingFrequency,
      false
    );
  }
}

void URedwoodClientInterface::HandlePingResult(
  FString TargetAddress, float RTT
) {
  URedwoodSettings *RedwoodSettings = GetMutableDefault<URedwoodSettings>();

  for (auto Itr : Regions) {
    if (Itr.Value->Url == TargetAddress) {
      Itr.Value->RTTs.Add(RTT);
      break;
    }
  }

  for (auto Itr : Regions) {
    if (Itr.Value->Url.StartsWith("ws")) {
      // the LatencyChecker module handles averaging for us for websockets
      // and sends PingAttempts and provides a single number
      if (Itr.Value->RTTs.Num() == 0) {
        // we haven't finished receiving all of the pings for this round
        return;
      }
    } else {
      if (Itr.Value->RTTs.Num() + PingAttemptsLeft != RedwoodSettings->PingAttempts) {
        // we haven't finished receiving all of the pings for this round
        return;
      }
    }
  }

  // we've received pings from all Regions, ping again
  InitiatePings();
}

void URedwoodClientInterface::Register(
  const FString &Username,
  const FString &Password,
  FRedwoodAuthUpdateDelegate OnUpdate
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodAuthUpdate Update;
    Update.Type = ERedwoodAuthUpdateType::Error;
    Update.Message = TEXT("Not connected to Director.");
    OnUpdate.ExecuteIfBound(Update);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("username"), Username);
  Payload->SetStringField(TEXT("password"), Password);

  Director->Emit(
    TEXT("player:register:username"),
    Payload,
    [this, OnUpdate](auto Response) {
      TSharedPtr<FJsonObject> MessageStruct = Response[0]->AsObject();
      PlayerId = MessageStruct->GetStringField(TEXT("playerId"));
      FString Error = MessageStruct->GetStringField(TEXT("error"));

      FRedwoodAuthUpdate Update;

      if (Error.IsEmpty()) {
        Update.Type = ERedwoodAuthUpdateType::Success;
        Update.Message = TEXT("");
      } else if (Error == "Must verify account") {
        OnAccountVerified = OnUpdate;
        Update.Type = ERedwoodAuthUpdateType::MustVerifyAccount;
        Update.Message = TEXT("");
      } else {
        Update.Type = ERedwoodAuthUpdateType::Error;
        Update.Message = Error;
      }

      OnUpdate.ExecuteIfBound(Update);
    }
  );
}

void URedwoodClientInterface::Logout() {
  if (IsLoggedIn()) {
    TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
    Payload->SetStringField(TEXT("playerId"), PlayerId);

    if (Director.IsValid() && Director->bIsConnected) {
      Director->Emit(TEXT("player:logout"), Payload);
    }

    if (Realm.IsValid() && Realm->bIsConnected) {
      Realm->Emit(TEXT("realm:auth:player:logout"), Payload);
      Realm->Disconnect();
    }

    PlayerId = TEXT("");
    AuthToken = TEXT("");

    URedwoodSaveGame *SaveGame = Cast<URedwoodSaveGame>(
      UGameplayStatics::CreateSaveGameObject(URedwoodSaveGame::StaticClass())
    );

    UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("RedwoodSaveGame"), 0);
  }
}

bool URedwoodClientInterface::IsLoggedIn() {
  return !PlayerId.IsEmpty() && !AuthToken.IsEmpty() && bAuthenticated;
}

FString URedwoodClientInterface::GetPlayerId() {
  return PlayerId;
}

FString URedwoodClientInterface::GetCharacterId() {
  return SelectedCharacterId;
}

FString URedwoodClientInterface::GetCharacterName() {
  if (!SelectedCharacterId.IsEmpty()) {
    FString *CharacterName = CharacterNamesById.Find(SelectedCharacterId);

    if (CharacterName != nullptr) {
      return *CharacterName;
    }
  }

  return FString();
}

FString URedwoodClientInterface::GetRealmId() {
  return CurrentRealmId;
}

void URedwoodClientInterface::AttemptAutoLogin(
  FRedwoodAuthUpdateDelegate OnUpdate
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodAuthUpdate Update;
    Update.Type = ERedwoodAuthUpdateType::Error;
    Update.Message = TEXT("Not connected to Director.");
    OnUpdate.ExecuteIfBound(Update);
    return;
  }

  URedwoodSaveGame *SaveGame = Cast<URedwoodSaveGame>(
    UGameplayStatics::LoadGameFromSlot(TEXT("RedwoodSaveGame"), 0)
  );

  if (SaveGame && !SaveGame->Username.IsEmpty() && !SaveGame->AuthToken.IsEmpty()) {
    Login(SaveGame->Username, SaveGame->AuthToken, "local", true, OnUpdate);
  } else {
    FRedwoodAuthUpdate Update;
    Update.Type = ERedwoodAuthUpdateType::Error;
    Update.Message = TEXT("No saved credentials found.");
    OnUpdate.ExecuteIfBound(Update);
  }
}

void URedwoodClientInterface::Login(
  const FString &Username,
  const FString &PasswordOrToken,
  const FString &Provider,
  bool bRememberMe,
  FRedwoodAuthUpdateDelegate OnUpdate,
  bool bBypassProviderCheck
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodAuthUpdate Update;
    Update.Type = ERedwoodAuthUpdateType::Error;
    Update.Message = TEXT("Not connected to Director.");
    OnUpdate.ExecuteIfBound(Update);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("username"), Username);
  Payload->SetStringField(TEXT("secret"), PasswordOrToken);
  Payload->SetStringField(TEXT("provider"), Provider);

  if (bBypassProviderCheck) {
    Payload->SetBoolField(TEXT("bypassProviderCheck"), true);
  }

  Director->Emit(
    TEXT("player:login:username"),
    Payload,
    [this, Username, bRememberMe, OnUpdate](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      PlayerId = MessageObject->GetStringField(TEXT("playerId"));
      AuthToken = MessageObject->GetStringField(TEXT("token"));
      Nickname = MessageObject->GetStringField(TEXT("nickname"));

      FRedwoodAuthUpdate Update;

      if (Error.IsEmpty()) {
        Update.Type = ERedwoodAuthUpdateType::Success;
        Update.Message = TEXT("");

        bAuthenticated = true;

        URedwoodSaveGame *SaveGame =
          Cast<URedwoodSaveGame>(UGameplayStatics::CreateSaveGameObject(
            URedwoodSaveGame::StaticClass()
          ));

        if (bRememberMe) {
          SaveGame->Username = Username;
          SaveGame->AuthToken = AuthToken;
        }

        UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("RedwoodSaveGame"), 0);
      } else if (Error == "Must verify account") {
        OnAccountVerified = OnUpdate;
        Update.Type = ERedwoodAuthUpdateType::MustVerifyAccount;
        Update.Message = TEXT("");
      } else {
        Update.Type = ERedwoodAuthUpdateType::Error;
        Update.Message = Error;

        URedwoodSaveGame *SaveGame =
          Cast<URedwoodSaveGame>(UGameplayStatics::CreateSaveGameObject(
            URedwoodSaveGame::StaticClass()
          ));
        UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("RedwoodSaveGame"), 0);
      }

      OnUpdate.ExecuteIfBound(Update);
    }
  );
}

void URedwoodClientInterface::LoginWithDiscord(
  bool bRememberMe, FRedwoodAuthUpdateDelegate OnUpdate
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodAuthUpdate Update;
    Update.Type = ERedwoodAuthUpdateType::Error;
    Update.Message = TEXT("Not connected to Director.");
    OnUpdate.ExecuteIfBound(Update);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);

  Director->Emit(
    TEXT("player:login:discord:initialize"),
    Payload,
    [this, bRememberMe, OnUpdate](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      if (Error.IsEmpty()) {
        FString ClientId = MessageObject->GetStringField(TEXT("clientId"));
        FString State = MessageObject->GetStringField(TEXT("state"));
        FString RedirectUri =
          MessageObject->GetStringField(TEXT("redirectUri"));

        FString AuthorizationUrl = FString::Printf(
          TEXT(
            "https://discord.com/oauth2/authorize?response_type=code&client_id=%s&scope=identify&state=%s&redirect_uri=%s&prompt=none&integration_type=1"
          ),
          *ClientId,
          *State,
          *RedirectUri
        );

        // open the browser
        FPlatformProcess::LaunchURL(*AuthorizationUrl, nullptr, nullptr);

        TSharedPtr<FJsonObject> FinalizePayload =
          MakeShareable(new FJsonObject);
        FinalizePayload->SetStringField(TEXT("state"), State);

        Director->Emit(
          TEXT("player:login:discord:finalize"),
          FinalizePayload,
          [this, bRememberMe, OnUpdate](auto FinalResponse) {
            TSharedPtr<FJsonObject> FinalMessageObject =
              FinalResponse[0]->AsObject();
            FString FinalError =
              FinalMessageObject->GetStringField(TEXT("error"));

            FRedwoodAuthUpdate Update;

            if (FinalError.IsEmpty()) {
              Update.Type = ERedwoodAuthUpdateType::Success;
              Update.Message = TEXT("");

              bAuthenticated = true;
              PlayerId = FinalMessageObject->GetStringField(TEXT("playerId"));
              AuthToken = FinalMessageObject->GetStringField(TEXT("token"));
              Nickname = FinalMessageObject->GetStringField(TEXT("nickname"));

              URedwoodSaveGame *SaveGame =
                Cast<URedwoodSaveGame>(UGameplayStatics::CreateSaveGameObject(
                  URedwoodSaveGame::StaticClass()
                ));

              if (bRememberMe) {
                SaveGame->Username =
                  FinalMessageObject->GetStringField(TEXT("username"));
                SaveGame->AuthToken = AuthToken;
              }

              UGameplayStatics::SaveGameToSlot(
                SaveGame, TEXT("RedwoodSaveGame"), 0
              );
            } else {
              Update.Type = ERedwoodAuthUpdateType::Error;
              Update.Message = FinalError;
            }

            OnUpdate.ExecuteIfBound(Update);
          }
        );
      } else {
        FRedwoodAuthUpdate Update;
        Update.Type = ERedwoodAuthUpdateType::Error;
        Update.Message = Error;
        OnUpdate.ExecuteIfBound(Update);
      }
    }
  );
}

void URedwoodClientInterface::LoginWithTwitch(
  bool bRememberMe, FRedwoodAuthUpdateDelegate OnUpdate
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodAuthUpdate Update;
    Update.Type = ERedwoodAuthUpdateType::Error;
    Update.Message = TEXT("Not connected to Director.");
    OnUpdate.ExecuteIfBound(Update);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);

  Director->Emit(
    TEXT("player:login:twitch:initialize"),
    Payload,
    [this, bRememberMe, OnUpdate](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      if (Error.IsEmpty()) {
        FString ClientId = MessageObject->GetStringField(TEXT("clientId"));
        FString State = MessageObject->GetStringField(TEXT("state"));
        FString RedirectUri =
          MessageObject->GetStringField(TEXT("redirectUri"));

        FString AuthorizationUrl = FString::Printf(
          TEXT(
            "https://id.twitch.tv/oauth2/authorize?response_type=code&client_id=%s&state=%s&redirect_uri=%s&scope="
          ),
          *ClientId,
          *State,
          *RedirectUri
        );

        // open the browser
        FPlatformProcess::LaunchURL(*AuthorizationUrl, nullptr, nullptr);

        TSharedPtr<FJsonObject> FinalizePayload =
          MakeShareable(new FJsonObject);
        FinalizePayload->SetStringField(TEXT("state"), State);

        Director->Emit(
          TEXT("player:login:twitch:finalize"),
          FinalizePayload,
          [this, bRememberMe, OnUpdate](auto FinalResponse) {
            TSharedPtr<FJsonObject> FinalMessageObject =
              FinalResponse[0]->AsObject();
            FString FinalError =
              FinalMessageObject->GetStringField(TEXT("error"));

            FRedwoodAuthUpdate Update;

            if (FinalError.IsEmpty()) {
              Update.Type = ERedwoodAuthUpdateType::Success;
              Update.Message = TEXT("");

              bAuthenticated = true;
              PlayerId = FinalMessageObject->GetStringField(TEXT("playerId"));
              AuthToken = FinalMessageObject->GetStringField(TEXT("token"));
              Nickname = FinalMessageObject->GetStringField(TEXT("nickname"));

              URedwoodSaveGame *SaveGame =
                Cast<URedwoodSaveGame>(UGameplayStatics::CreateSaveGameObject(
                  URedwoodSaveGame::StaticClass()
                ));

              if (bRememberMe) {
                SaveGame->Username =
                  FinalMessageObject->GetStringField(TEXT("username"));
                SaveGame->AuthToken = AuthToken;
              }

              UGameplayStatics::SaveGameToSlot(
                SaveGame, TEXT("RedwoodSaveGame"), 0
              );
            } else {
              Update.Type = ERedwoodAuthUpdateType::Error;
              Update.Message = FinalError;
            }

            OnUpdate.ExecuteIfBound(Update);
          }
        );
      } else {
        FRedwoodAuthUpdate Update;
        Update.Type = ERedwoodAuthUpdateType::Error;
        Update.Message = Error;
        OnUpdate.ExecuteIfBound(Update);
      }
    }
  );
}

FString URedwoodClientInterface::GetNickname() {
  return Nickname;
}

void URedwoodClientInterface::CancelWaitingForAccountVerification() {
  if (OnAccountVerified.IsBound()) {
    OnAccountVerified.Unbind();
  }
}

void URedwoodClientInterface::SearchForPlayers(
  FString UsernameOrNickname,
  bool bIncludePartialMatches,
  FRedwoodListPlayersOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodListPlayersOutput Output;
    Output.Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("searchText"), UsernameOrNickname);
  Payload->SetBoolField(TEXT("includePartial"), bIncludePartialMatches);

  Director->Emit(
    TEXT("director:players:search:name"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FRedwoodListPlayersOutput Output;

      Output.Error = MessageObject->GetStringField(TEXT("error"));

      const TArray<TSharedPtr<FJsonValue>> &Players =
        MessageObject->GetArrayField(TEXT("players"));

      for (TSharedPtr<FJsonValue> InPlayer : Players) {
        FRedwoodPlayer OutPlayer;
        TSharedPtr<FJsonObject> FriendObj = InPlayer->AsObject();
        OutPlayer.PlayerId = FriendObj->GetStringField(TEXT("playerId"));
        OutPlayer.Nickname = FriendObj->GetStringField(TEXT("nickname"));
        OutPlayer.FriendshipState =
          URedwoodCommonGameSubsystem::ParseFriendListType(
            FriendObj->GetStringField(TEXT("friendshipState"))
          );

        const TSharedPtr<FJsonObject> *OnlineStateObj;
        OutPlayer.bOnline =
          FriendObj->TryGetObjectField(TEXT("onlineState"), OnlineStateObj);
        if (OutPlayer.bOnline) {

          const TSharedPtr<FJsonObject> *OnlineStateRealmObj;
          OutPlayer.bPlaying =
            (*OnlineStateObj)
              ->TryGetObjectField(TEXT("realm"), OnlineStateRealmObj);

          if (OutPlayer.bPlaying) {
            OutPlayer.OnlineStateRealm.RealmName =
              (*OnlineStateRealmObj)->GetStringField(TEXT("realmName"));
            OutPlayer.OnlineStateRealm.ProxyId =
              (*OnlineStateRealmObj)->GetStringField(TEXT("proxyId"));
            OutPlayer.OnlineStateRealm.ZoneName =
              (*OnlineStateRealmObj)->GetStringField(TEXT("zoneName"));
            OutPlayer.OnlineStateRealm.ShardName =
              (*OnlineStateRealmObj)->GetStringField(TEXT("shardName"));
            OutPlayer.OnlineStateRealm.CharacterId =
              (*OnlineStateRealmObj)->GetStringField(TEXT("characterId"));
          }
        }

        Output.Players.Add(OutPlayer);
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::SearchForPlayerById(
  FString TargetPlayerId, FRedwoodPlayerOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodPlayerOutput Output;
    Output.Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("id"), PlayerId);
  Payload->SetStringField(TEXT("targetPlayerId"), TargetPlayerId);

  Director->Emit(
    TEXT("director:players:search:id"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FRedwoodPlayerOutput Output;

      Output.Error = MessageObject->GetStringField(TEXT("error"));

      const TSharedPtr<FJsonObject> *PlayerObject = nullptr;
      if (MessageObject->TryGetObjectField(TEXT("player"), PlayerObject)) {
        FRedwoodPlayer OutPlayer;
        OutPlayer.PlayerId = (*PlayerObject)->GetStringField(TEXT("playerId"));
        OutPlayer.Nickname = (*PlayerObject)->GetStringField(TEXT("nickname"));
        OutPlayer.FriendshipState =
          URedwoodCommonGameSubsystem::ParseFriendListType(
            (*PlayerObject)->GetStringField(TEXT("friendshipState"))
          );

        const TSharedPtr<FJsonObject> *OnlineStateObj;
        OutPlayer.bOnline =
          (*PlayerObject)
            ->TryGetObjectField(TEXT("onlineState"), OnlineStateObj);
        if (OutPlayer.bOnline) {

          const TSharedPtr<FJsonObject> *OnlineStateRealmObj;
          OutPlayer.bPlaying =
            (*OnlineStateObj)
              ->TryGetObjectField(TEXT("realm"), OnlineStateRealmObj);

          if (OutPlayer.bPlaying) {
            OutPlayer.OnlineStateRealm.RealmName =
              (*OnlineStateRealmObj)->GetStringField(TEXT("realmName"));
            OutPlayer.OnlineStateRealm.ProxyId =
              (*OnlineStateRealmObj)->GetStringField(TEXT("proxyId"));
            OutPlayer.OnlineStateRealm.ZoneName =
              (*OnlineStateRealmObj)->GetStringField(TEXT("zoneName"));
            OutPlayer.OnlineStateRealm.ShardName =
              (*OnlineStateRealmObj)->GetStringField(TEXT("shardName"));
            OutPlayer.OnlineStateRealm.CharacterId =
              (*OnlineStateRealmObj)->GetStringField(TEXT("characterId"));
          }
        }

        Output.Player = OutPlayer;
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::ListFriends(
  ERedwoodFriendListType Filter, FRedwoodListPlayersOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodListPlayersOutput Output;
    Output.Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  if (Filter == ERedwoodFriendListType::All || Filter == ERedwoodFriendListType::Unknown) {
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());
    Payload->SetField(TEXT("filter"), NullValue);
  } else {
    Payload->SetStringField(TEXT("filter"), RW_ENUM_TO_STRING(Filter));
  }

  Director->Emit(
    TEXT("director:friends:list"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FRedwoodListPlayersOutput Output;

      Output.Error = MessageObject->GetStringField(TEXT("error"));

      const TArray<TSharedPtr<FJsonValue>> &Players =
        MessageObject->GetArrayField(TEXT("players"));

      for (TSharedPtr<FJsonValue> InPlayer : Players) {
        FRedwoodPlayer OutPlayer;
        TSharedPtr<FJsonObject> FriendObj = InPlayer->AsObject();
        OutPlayer.PlayerId = FriendObj->GetStringField(TEXT("playerId"));
        OutPlayer.Nickname = FriendObj->GetStringField(TEXT("nickname"));
        OutPlayer.FriendshipState =
          URedwoodCommonGameSubsystem::ParseFriendListType(
            FriendObj->GetStringField(TEXT("friendshipState"))
          );

        const TSharedPtr<FJsonObject> *OnlineStateObj;
        OutPlayer.bOnline =
          FriendObj->TryGetObjectField(TEXT("onlineState"), OnlineStateObj);
        if (OutPlayer.bOnline) {

          const TSharedPtr<FJsonObject> *OnlineStateRealmObj;
          OutPlayer.bPlaying =
            (*OnlineStateObj)
              ->TryGetObjectField(TEXT("realm"), OnlineStateRealmObj);

          if (OutPlayer.bPlaying) {
            OutPlayer.OnlineStateRealm.RealmName =
              (*OnlineStateRealmObj)->GetStringField(TEXT("realmName"));
            OutPlayer.OnlineStateRealm.ProxyId =
              (*OnlineStateRealmObj)->GetStringField(TEXT("proxyId"));
            OutPlayer.OnlineStateRealm.ZoneName =
              (*OnlineStateRealmObj)->GetStringField(TEXT("zoneName"));
            OutPlayer.OnlineStateRealm.ShardName =
              (*OnlineStateRealmObj)->GetStringField(TEXT("shardName"));
            OutPlayer.OnlineStateRealm.CharacterId =
              (*OnlineStateRealmObj)->GetStringField(TEXT("characterId"));
          }
        }

        Output.Players.Add(OutPlayer);
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::RequestFriend(
  FString OtherPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("friendId"), OtherPlayerId);

  Director->Emit(
    TEXT("director:friends:add"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::RemoveFriend(
  FString OtherPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("friendId"), OtherPlayerId);

  Director->Emit(
    TEXT("director:friends:remove"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::RespondToFriendRequest(
  FString OtherPlayerId, bool bAccept, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("friendId"), OtherPlayerId);
  Payload->SetBoolField(TEXT("accept"), bAccept);

  Director->Emit(
    TEXT("director:friends:respond-request"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::SetPlayerBlocked(
  FString OtherPlayerId, bool bBlocked, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("friendId"), OtherPlayerId);
  Payload->SetBoolField(TEXT("block"), bBlocked);

  Director->Emit(
    TEXT("director:friends:block"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::ListGuilds(
  bool bOnlyPlayersGuilds, FRedwoodListGuildsOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodListGuildsOutput Output;
    Output.Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetBoolField(TEXT("onlyPlayersGuilds"), bOnlyPlayersGuilds);

  Director->Emit(
    TEXT("director:guilds:list"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FRedwoodListGuildsOutput Output;

      Output.Error = MessageObject->GetStringField(TEXT("error"));

      if (Output.Error.IsEmpty()) {
        const TArray<TSharedPtr<FJsonValue>> *GuildsArray;
        if (MessageObject->TryGetArrayField(TEXT("guilds"), GuildsArray)) {
          for (const TSharedPtr<FJsonValue> &GuildValue : *GuildsArray) {
            TSharedPtr<FJsonObject> GuildInfoObject = GuildValue->AsObject();

            Output.Guilds.Add(
              URedwoodCommonGameSubsystem::ParseGuildInfo(GuildInfoObject)
            );
          }
        }
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::SearchForGuilds(
  FString SearchText,
  bool bIncludePartialMatches,
  FRedwoodListGuildsOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodListGuildsOutput Output;
    Output.Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("searchText"), SearchText);
  Payload->SetBoolField(TEXT("includePartial"), bIncludePartialMatches);

  Director->Emit(
    TEXT("director:guilds:search"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FRedwoodListGuildsOutput Output;

      Output.Error = MessageObject->GetStringField(TEXT("error"));

      if (Output.Error.IsEmpty()) {
        const TArray<TSharedPtr<FJsonValue>> *GuildsArray;
        if (MessageObject->TryGetArrayField(TEXT("guilds"), GuildsArray)) {
          for (const TSharedPtr<FJsonValue> &GuildValue : *GuildsArray) {
            TSharedPtr<FJsonObject> GuildInfoObject = GuildValue->AsObject();

            Output.Guilds.Add(
              URedwoodCommonGameSubsystem::ParseGuildInfo(GuildInfoObject)
            );
          }
        }
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::GetGuild(
  FString GuildId, FRedwoodGetGuildOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodGetGuildOutput Output;
    Output.Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("guildId"), GuildId);

  Director->Emit(
    TEXT("director:guilds:get"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FRedwoodGetGuildOutput Output;

      Output.Error = MessageObject->GetStringField(TEXT("error"));

      if (Output.Error.IsEmpty()) {
        TSharedPtr<FJsonObject> GuildObject =
          MessageObject->GetObjectField(TEXT("guild"));
        if (GuildObject) {
          Output.Guild =
            URedwoodCommonGameSubsystem::ParseGuildInfo(GuildObject);
        }
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::GetSelectedGuild(
  FRedwoodGetGuildOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodGetGuildOutput Output;
    Output.Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  Director->Emit(
    TEXT("director:guilds:selected:get"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FRedwoodGetGuildOutput Output;

      Output.Error = MessageObject->GetStringField(TEXT("error"));

      if (Output.Error.IsEmpty()) {
        TSharedPtr<FJsonObject> GuildObject =
          MessageObject->GetObjectField(TEXT("guild"));
        if (GuildObject) {
          Output.Guild =
            URedwoodCommonGameSubsystem::ParseGuildInfo(GuildObject);
        }
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::SetSelectedGuild(
  FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("guildId"), GuildId);

  Director->Emit(
    TEXT("director:guilds:selected:set"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::JoinGuild(
  FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("guildId"), GuildId);

  Director->Emit(
    TEXT("director:guilds:membership:join"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::InviteToGuild(
  FString GuildId, FString TargetId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("guildId"), GuildId);
  Payload->SetStringField(TEXT("targetId"), TargetId);

  Director->Emit(
    TEXT("director:guilds:membership:invite"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::LeaveGuild(
  FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("guildId"), GuildId);

  Director->Emit(
    TEXT("director:guilds:membership:leave"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::ListGuildMembers(
  FString GuildId,
  ERedwoodGuildAndAllianceMemberState State,
  FRedwoodListGuildMembersOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodListGuildMembersOutput Output;
    Output.Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("guildId"), GuildId);
  Payload->SetStringField(
    TEXT("state"),
    URedwoodCommonGameSubsystem::SerializeGuildAndAllianceMemberState(State)
  );

  Director->Emit(
    TEXT("director:guilds:membership:list"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FRedwoodListGuildMembersOutput Output;

      Output.Error = MessageObject->GetStringField(TEXT("error"));

      const TArray<TSharedPtr<FJsonValue>> &Members =
        MessageObject->GetArrayField(TEXT("members"));

      for (TSharedPtr<FJsonValue> InMember : Members) {
        FRedwoodGuildPlayerMembership OutMember;
        TSharedPtr<FJsonObject> MemberObj = InMember->AsObject();
        if (!MemberObj.IsValid()) {
          continue; // skip invalid members
        }
        TSharedPtr<FJsonObject> PlayerObj =
          MemberObj->GetObjectField(TEXT("player"));
        if (!PlayerObj.IsValid()) {
          continue; // skip members without player info
        }
        OutMember.Player.Id = PlayerObj->GetStringField(TEXT("id"));
        OutMember.Player.Name = PlayerObj->GetStringField(TEXT("name"));
        OutMember.PlayerState =
          URedwoodCommonGameSubsystem::ParseGuildAndAllianceMemberState(
            MemberObj->GetStringField(TEXT("playerState"))
          );

        Output.Members.Add(OutMember);
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::CreateGuild(
  FString GuildName,
  FString GuildTag,
  ERedwoodGuildInviteType InviteType,
  bool bListed,
  bool bMembershipPublic,
  FRedwoodCreateGuildOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodCreateGuildOutput Output;
    Output.Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("name"), GuildName);
  Payload->SetStringField(TEXT("tag"), GuildTag);
  Payload->SetStringField(
    TEXT("inviteType"),
    URedwoodCommonGameSubsystem::SerializeGuildInviteType(InviteType)
  );
  Payload->SetBoolField(TEXT("listed"), bListed);
  Payload->SetBoolField(TEXT("membershipPublic"), bMembershipPublic);

  Director->Emit(
    TEXT("director:guilds:admin:create"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FRedwoodCreateGuildOutput Output;

      Output.Error = MessageObject->GetStringField(TEXT("error"));
      MessageObject->TryGetStringField(TEXT("guildId"), Output.GuildId);

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::UpdateGuild(
  FString GuildId,
  FString GuildName,
  FString GuildTag,
  ERedwoodGuildInviteType InviteType,
  bool bListed,
  bool bMembershipPublic,
  FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("guildId"), GuildId);
  Payload->SetStringField(TEXT("name"), GuildName);
  Payload->SetStringField(TEXT("tag"), GuildTag);
  Payload->SetStringField(
    TEXT("inviteType"),
    URedwoodCommonGameSubsystem::SerializeGuildInviteType(InviteType)
  );
  Payload->SetBoolField(TEXT("listed"), bListed);
  Payload->SetBoolField(TEXT("membershipPublic"), bMembershipPublic);

  Director->Emit(
    TEXT("director:guilds:admin:update"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::KickPlayerFromGuild(
  FString GuildId, FString TargetId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("guildId"), GuildId);
  Payload->SetStringField(TEXT("targetId"), TargetId);
  Payload->SetBoolField(TEXT("ban"), false);

  Director->Emit(
    TEXT("director:guilds:admin:kick"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::BanPlayerFromGuild(
  FString GuildId, FString TargetId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("guildId"), GuildId);
  Payload->SetStringField(TEXT("targetId"), TargetId);
  Payload->SetBoolField(TEXT("ban"), true);

  Director->Emit(
    TEXT("director:guilds:admin:kick"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::UnbanPlayerFromGuild(
  FString GuildId, FString TargetPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  KickPlayerFromGuild(GuildId, TargetPlayerId, OnOutput);
}

void URedwoodClientInterface::PromotePlayerToGuildAdmin(
  FString GuildId, FString TargetId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("guildId"), GuildId);
  Payload->SetStringField(TEXT("targetId"), TargetId);

  Director->Emit(
    TEXT("director:guilds:admin:promote"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::DemotePlayerFromGuildAdmin(
  FString GuildId, FString TargetId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("guildId"), GuildId);
  Payload->SetStringField(TEXT("targetId"), TargetId);

  Director->Emit(
    TEXT("director:guilds:admin:demote"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::ListAlliances(
  FString GuildIdFilter, FRedwoodListAlliancesOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodListAlliancesOutput Output;
    Output.Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("guildIdFilter"), GuildIdFilter);

  Director->Emit(
    TEXT("director:guilds:alliances:list"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FRedwoodListAlliancesOutput Output;

      Output.Error = MessageObject->GetStringField(TEXT("error"));

      const TArray<TSharedPtr<FJsonValue>> &Alliances =
        MessageObject->GetArrayField(TEXT("alliances"));

      for (TSharedPtr<FJsonValue> InAlliance : Alliances) {
        TSharedPtr<FJsonObject> AllianceObj = InAlliance->AsObject();

        Output.Alliances.Add(
          URedwoodCommonGameSubsystem::ParseAlliance(AllianceObj)
        );
      }

      // check if guildStates is not null
      const TArray<TSharedPtr<FJsonValue>> *GuildStates;
      if (MessageObject->TryGetArrayField(TEXT("guildStates"), GuildStates)) {
        for (const TSharedPtr<FJsonValue> &InState : *GuildStates) {
          Output.GuildStates.Add(
            URedwoodCommonGameSubsystem::ParseGuildAndAllianceMemberState(
              InState->AsString()
            )
          );
        }
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::SearchForAlliances(
  FString SearchText,
  bool bIncludePartialMatches,
  FRedwoodListAlliancesOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodListAlliancesOutput Output;
    Output.Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("searchText"), SearchText);
  Payload->SetBoolField(TEXT("includePartial"), bIncludePartialMatches);

  Director->Emit(
    TEXT("director:guilds:alliances:search"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FRedwoodListAlliancesOutput Output;

      Output.Error = MessageObject->GetStringField(TEXT("error"));

      const TArray<TSharedPtr<FJsonValue>> &Alliances =
        MessageObject->GetArrayField(TEXT("alliances"));

      for (TSharedPtr<FJsonValue> InAlliance : Alliances) {
        TSharedPtr<FJsonObject> AllianceObj = InAlliance->AsObject();

        Output.Alliances.Add(
          URedwoodCommonGameSubsystem::ParseAlliance(AllianceObj)
        );
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::CanAdminAlliance(
  FString AllianceId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    OnOutput.ExecuteIfBound(TEXT("Not connected to Director."));
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("allianceId"), AllianceId);

  Director->Emit(
    TEXT("director:guilds:alliances:admin:has-admin-privileges"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::CreateAlliance(
  FString AllianceName,
  FString GuildId,
  bool bInviteOnly,
  FRedwoodCreateAllianceOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodCreateAllianceOutput Output;
    Output.Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("guildId"), GuildId);
  Payload->SetStringField(TEXT("name"), AllianceName);
  Payload->SetBoolField(TEXT("inviteOnly"), bInviteOnly);

  Director->Emit(
    TEXT("director:guilds:alliances:admin:create"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FRedwoodCreateAllianceOutput Output;

      Output.Error = MessageObject->GetStringField(TEXT("error"));
      MessageObject->TryGetStringField(TEXT("allianceId"), Output.AllianceId);

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::UpdateAlliance(
  FString AllianceId,
  FString AllianceName,
  bool bInviteOnly,
  FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("allianceId"), AllianceId);
  Payload->SetStringField(TEXT("name"), AllianceName);
  Payload->SetBoolField(TEXT("inviteOnly"), bInviteOnly);

  Director->Emit(
    TEXT("director:guilds:alliances:admin:update"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::KickGuildFromAlliance(
  FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("allianceId"), AllianceId);
  Payload->SetStringField(TEXT("targetGuildId"), GuildId);
  Payload->SetBoolField(TEXT("ban"), false);

  Director->Emit(
    TEXT("director:guilds:alliances:admin:kick"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::BanGuildFromAlliance(
  FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("allianceId"), AllianceId);
  Payload->SetStringField(TEXT("guildId"), GuildId);
  Payload->SetBoolField(TEXT("ban"), true);

  Director->Emit(
    TEXT("director:guilds:alliances:admin:kick"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::UnbanGuildFromAlliance(
  FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  KickGuildFromAlliance(AllianceId, GuildId, OnOutput);
}

void URedwoodClientInterface::ListAllianceGuilds(
  FString AllianceId,
  ERedwoodGuildAndAllianceMemberState State,
  FRedwoodListAllianceGuildsOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodListAllianceGuildsOutput Output;
    Output.Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("allianceId"), AllianceId);
  Payload->SetStringField(
    TEXT("state"),
    URedwoodCommonGameSubsystem::SerializeGuildAndAllianceMemberState(State)
  );

  Director->Emit(
    TEXT("director:guilds:alliances:membership:list"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FRedwoodListAllianceGuildsOutput Output;

      Output.Error = MessageObject->GetStringField(TEXT("error"));

      const TArray<TSharedPtr<FJsonValue>> &Guilds =
        MessageObject->GetArrayField(TEXT("guilds"));

      for (TSharedPtr<FJsonValue> InGuild : Guilds) {
        FRedwoodAllianceGuildMembership OutGuild;
        TSharedPtr<FJsonObject> GuildMembershipObj = InGuild->AsObject();
        if (!GuildMembershipObj.IsValid()) {
          continue; // skip invalid guilds
        }
        TSharedPtr<FJsonObject> GuildObj =
          GuildMembershipObj->GetObjectField(TEXT("guild"));
        if (!GuildObj.IsValid()) {
          continue; // skip guilds without player info
        }
        OutGuild.Guild = URedwoodCommonGameSubsystem::ParseGuild(GuildObj);

        OutGuild.GuildState =
          URedwoodCommonGameSubsystem::ParseGuildAndAllianceMemberState(
            GuildMembershipObj->GetStringField(TEXT("guildState"))
          );

        Output.Guilds.Add(OutGuild);
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::JoinAlliance(
  FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("allianceId"), AllianceId);
  Payload->SetStringField(TEXT("guildId"), GuildId);

  Director->Emit(
    TEXT("director:guilds:alliances:membership:join"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::LeaveAlliance(
  FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("allianceId"), AllianceId);
  Payload->SetStringField(TEXT("guildId"), GuildId);

  Director->Emit(
    TEXT("director:guilds:alliances:membership:leave"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::InviteGuildToAlliance(
  FString AllianceId, FString GuildId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FString Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("allianceId"), AllianceId);
  Payload->SetStringField(TEXT("targetGuildId"), GuildId);

  Director->Emit(
    TEXT("director:guilds:alliances:membership:invite"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::ListRealms(
  FRedwoodListRealmsOutputDelegate OnOutput
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodListRealmsOutput Output;
    Output.Error = TEXT("Not connected to Director.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  Director->Emit(TEXT("realm:list"), Payload, [this, OnOutput](auto Response) {
    TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

    FRedwoodListRealmsOutput Output;

    Output.Error = MessageObject->GetStringField(TEXT("error"));

    const TArray<TSharedPtr<FJsonValue>> &Realms =
      MessageObject->GetArrayField(TEXT("realms"));

    for (TSharedPtr<FJsonValue> InRealm : Realms) {
      FRedwoodRealm OutRealm;
      TSharedPtr<FJsonObject> RealmObj = InRealm->AsObject();
      OutRealm.Id = RealmObj->GetStringField(TEXT("id"));
      FDateTime::ParseIso8601(
        *RealmObj->GetStringField(TEXT("createdAt")), OutRealm.CreatedAt
      );
      FDateTime::ParseIso8601(
        *RealmObj->GetStringField(TEXT("updatedAt")), OutRealm.UpdatedAt
      );
      OutRealm.Name = RealmObj->GetStringField(TEXT("name"));
      OutRealm.Uri = RealmObj->GetStringField(TEXT("uri"));
      OutRealm.bListed = RealmObj->GetBoolField(TEXT("listed"));
      OutRealm.Secret = RealmObj->GetStringField(TEXT("secret"));

      Output.Realms.Add(OutRealm);
    }

    OnOutput.ExecuteIfBound(Output);
  });
}

void URedwoodClientInterface::InitializeConnectionForFirstRealm(
  FRedwoodSocketConnectedDelegate OnRealmConnected
) {
  ListRealms(FRedwoodListRealmsOutputDelegate::CreateLambda(
    [this, OnRealmConnected](const FRedwoodListRealmsOutput &Output) {
      if (!Output.Error.IsEmpty()) {
        FRedwoodSocketConnected ConnectionResult;
        ConnectionResult.Error = Output.Error;
        OnRealmConnected.ExecuteIfBound(ConnectionResult);
        return;
      }

      if (Output.Realms.Num() == 0) {
        FRedwoodSocketConnected ConnectionResult;
        ConnectionResult.Error = TEXT("No realms found.");
        OnRealmConnected.ExecuteIfBound(ConnectionResult);
        return;
      }

      InitializeRealmConnection(
        Output.Realms[0],
        FRedwoodSocketConnectedDelegate::CreateLambda(
          [OnRealmConnected](const FRedwoodSocketConnected &ConnectionResult) {
            OnRealmConnected.ExecuteIfBound(ConnectionResult);
          }
        )
      );
    }
  ));
}

void URedwoodClientInterface::InitializeRealmConnection(
  FRedwoodRealm InRealm, FRedwoodSocketConnectedDelegate OnRealmConnected
) {
  InitiateRealmHandshake(InRealm, OnRealmConnected);
}

void URedwoodClientInterface::InitiateRealmHandshake(
  FRedwoodRealm InRealm, FRedwoodSocketConnectedDelegate OnRealmConnected
) {
  if (!Director.IsValid() || !Director->bIsConnected || !IsLoggedIn()) {
    FRedwoodSocketConnected Output;
    Output.Error = TEXT("Not connected or authenticated to Director.");
    OnRealmConnected.ExecuteIfBound(Output);
    return;
  }

  CurrentRealm = FRedwoodRealm();

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("realmId"), InRealm.Id);

  Director->Emit(
    TEXT("realm:auth:player:connect:client-to-director"),
    Payload,
    [this, InRealm, OnRealmConnected](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      FString Token = MessageObject->GetStringField(TEXT("token"));

      if (!Error.IsEmpty()) {
        FRedwoodSocketConnected Output;
        Output.Error = Error;
        OnRealmConnected.ExecuteIfBound(Output);
        return;
      }

      CurrentRealmId = InRealm.Id;
      CurrentRealm = InRealm;
      Realm = ISocketIOClientModule::Get().NewValidNativePointer();
      bSentRealmConnected = false;

      Realm->OnReconnectionCallback = [InRealm, this](
                                        unsigned ReconnectionAttempt,
                                        unsigned AttemptDelay
                                      ) {
        if (!bSentRealmConnected && !bSentInitialRealmConnectionFailureLog) {
          bSentInitialRealmConnectionFailureLog = true;
          UE_LOG(
            LogRedwood,
            Error,
            TEXT(
              "Unable to establish initial connection to Realm at %s; will continue to try to establish connection. See SocketIO plugin logs for retry attempts."
            ),
            *InRealm.Uri
          );
        } else if (!bRealmDisconnected) {
          bRealmDisconnected = true;
          UE_LOG(
            LogRedwood,
            Error,
            TEXT(
              "Lost connection to Realm at %s; will continue to try to reestablish connection. See SocketIO plugin logs for retry attempts."
            ),
            *InRealm.Uri
          );
          OnRealmConnectionLost.Broadcast();
        }
      };

      Realm->OnConnectedCallback = [this, Token, OnRealmConnected, InRealm](
                                     const FString &InSocketId,
                                     const FString &InSessionId
                                   ) {
        bRealmDisconnected = false;

        if (!bSentRealmConnected) {
          UE_LOG(
            LogRedwood, Log, TEXT("Connected to Realm at %s"), *InRealm.Uri
          );
          bSentRealmConnected = true;
          FinalizeRealmHandshake(Token, OnRealmConnected);
        } else {
          UE_LOG(
            LogRedwood,
            Log,
            TEXT(
              "Reestablished connection to Realm at %s, attempting to reauthenticate"
            ),
            *InRealm.Uri
          );
          BeginRealmReauthentication();
        }
      };

      UE_LOG(LogRedwood, Log, TEXT("Connecting to Realm at %s"), *InRealm.Uri);
      Realm->Connect(*InRealm.Uri);
    }
  );
}

void URedwoodClientInterface::BeginRealmReauthentication() {
  if (!Director.IsValid() || !Director->bIsConnected || !IsLoggedIn()) {
    TimerManager.SetTimer(
      ReauthenticationAttemptTimer,
      this,
      &URedwoodClientInterface::BeginRealmReauthentication,
      0.5f,
      false
    );
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("realmId"), CurrentRealmId);

  Director->Emit(
    TEXT("realm:auth:player:connect:client-to-director"),
    Payload,
    [this](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      FString Token = MessageObject->GetStringField(TEXT("token"));

      if (!Error.IsEmpty()) {
        UE_LOG(
          LogRedwood,
          Error,
          TEXT("Could not reauthenticate connection with Realm: %s"),
          *Error
        );
        return;
      }

      FinalizeRealmHandshake(
        Token,
        FRedwoodSocketConnectedDelegate::CreateLambda(
          [this](const FRedwoodSocketConnected &Output) {
            if (Output.Error.IsEmpty()) {
              UE_LOG(
                LogRedwood,
                Log,
                TEXT(
                  "Reauthenticated connection with Realm, calling connection reestablished."
                )
              );
              OnRealmConnectionReestablished.Broadcast();
            } else {
              UE_LOG(
                LogRedwood,
                Error,
                TEXT("Could not reauthenticate connection with Realm: %s"),
                *Output.Error
              );
            }
          }
        )
      );
    }
  );
}

void URedwoodClientInterface::FinalizeRealmHandshake(
  FString Token, FRedwoodSocketConnectedDelegate OnRealmConnected
) {
  BindRealmEvents();

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("token"), Token);

  Realm->Emit(
    TEXT("realm:auth:player:connect:client-to-realm"),
    Payload,
    [this, OnRealmConnected](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      FRedwoodSocketConnected Output;
      Output.Error = Error;
      OnRealmConnected.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::BindRealmEvents() {
  Realm->OnEvent(
    TEXT("realm:regions"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      HandleRegionsChanged(Event, Message);
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  Realm->OnEvent(
    TEXT("realm:ticketing:update"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      FRedwoodTicketingUpdate Update;
      Update.Type = ERedwoodTicketingUpdateType::Update;
      Update.Message = Message->AsObject()->GetStringField(TEXT("message"));
      OnTicketingUpdate.ExecuteIfBound(Update);
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  Realm->OnEvent(
    TEXT("realm:ticketing:ticket-error"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      FRedwoodTicketingUpdate Update;
      Update.Type = ERedwoodTicketingUpdateType::TicketError;
      Update.Message = Message->AsObject()->GetStringField(TEXT("error"));
      OnTicketingUpdate.ExecuteIfBound(Update);

      OnTicketingUpdate = FRedwoodTicketingUpdateDelegate();
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  Realm->OnEvent(
    TEXT("realm:servers:connect-to-instance"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      TSharedPtr<FJsonObject> MessageObject = Message->AsObject();

      bool bShouldStitch = MessageObject->GetBoolField(TEXT("shouldStitch"));
      ServerConnection = MessageObject->GetStringField(TEXT("connection"));
      ServerToken = MessageObject->GetStringField(TEXT("token"));

      if (bShouldStitch) {
        FURL URL = GetConnectionURL();
        OnRequestToStitchServer.Broadcast(URL);
      } else {
        FString ConsoleCommand = GetConnectionConsoleCommand();
        OnRequestToJoinServer.Broadcast(ConsoleCommand);
      }

      OnTicketingUpdate = FRedwoodTicketingUpdateDelegate();
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  Realm->OnEvent(
    TEXT("realm:parties:invites:alert"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      TSharedPtr<FJsonObject> MessageObject = Message->AsObject();

      TSharedPtr<FJsonObject> InviteObject =
        MessageObject->GetObjectField(TEXT("invite"));
      FRedwoodPartyInvite Invite =
        URedwoodCommonGameSubsystem::ParsePartyInvite(InviteObject);

      OnPartyInvited.Broadcast(Invite);
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  Realm->OnEvent(
    TEXT("realm:parties:changed"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      TSharedPtr<FJsonObject> MessageObject = Message->AsObject();

      TSharedPtr<FJsonObject> PartyObject =
        MessageObject->GetObjectField(TEXT("party"));
      CurrentParty = URedwoodCommonGameSubsystem::ParseParty(PartyObject);
      OnPartyUpdated.Broadcast(CurrentParty);
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  Realm->OnEvent(
    TEXT("realm:parties:kick"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      CurrentParty = FRedwoodParty();
      OnPartyKicked.Broadcast();
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  Realm->OnEvent(
    TEXT("realm:parties:emote"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      TSharedPtr<FJsonObject> MessageObject = Message->AsObject();

      FString TargetPlayerId = MessageObject->GetStringField(TEXT("playerId"));
      FString Emote = MessageObject->GetStringField(TEXT("emote"));

      OnPartyEmoteReceived.Broadcast(TargetPlayerId, Emote);
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );
}

bool URedwoodClientInterface::IsRealmConnected(FRedwoodRealm &OutRealm) {
  bool bIsConnected = IsRealmConnected();
  if (bIsConnected) {
    OutRealm = CurrentRealm;
  }
  return bIsConnected;
}

bool URedwoodClientInterface::IsRealmConnected() {
  return Realm.IsValid() && Realm->bIsConnected;
}

TMap<FString, float> URedwoodClientInterface::GetRegions() {
  return PingAverages;
}

void URedwoodClientInterface::ListCharacters(
  FRedwoodListCharactersOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodListCharactersOutput Output;
    Output.Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  Realm->Emit(
    TEXT("realm:characters:list"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      TArray<TSharedPtr<FJsonValue>> Characters =
        MessageObject->GetArrayField(TEXT("characters"));

      CharacterNamesById.Reset();
      TArray<FRedwoodCharacterBackend> CharactersStruct;
      for (TSharedPtr<FJsonValue> Character : Characters) {
        TSharedPtr<FJsonObject> CharacterData = Character->AsObject();

        FRedwoodCharacterBackend CharacterStruct =
          URedwoodCommonGameSubsystem::ParseCharacter(CharacterData);

        CharactersStruct.Add(CharacterStruct);

        CharacterNamesById.Add(CharacterStruct.Id, CharacterStruct.Name);
      }

      FRedwoodListCharactersOutput Output;
      Output.Error = Error;
      Output.Characters = CharactersStruct;
      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::ListArchivedCharacters(
  FRedwoodListCharactersOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodListCharactersOutput Output;
    Output.Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetBoolField(TEXT("includeArchived"), true);

  Realm->Emit(
    TEXT("realm:characters:list"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      TArray<TSharedPtr<FJsonValue>> Characters =
        MessageObject->GetArrayField(TEXT("characters"));

      TArray<FRedwoodCharacterBackend> CharactersStruct;
      for (TSharedPtr<FJsonValue> Character : Characters) {
        TSharedPtr<FJsonObject> CharacterData = Character->AsObject();

        FRedwoodCharacterBackend ParsedCharacter =
          URedwoodCommonGameSubsystem::ParseCharacter(CharacterData);
        if (ParsedCharacter.bArchived) {
          CharactersStruct.Add(ParsedCharacter);
        }
      }

      FRedwoodListCharactersOutput Output;
      Output.Error = Error;
      Output.Characters = CharactersStruct;
      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::CreateCharacter(
  FString Name,
  USIOJsonObject *CharacterCreatorData,
  FRedwoodGetCharacterOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodGetCharacterOutput Output;
    Output.Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  if (CharacterCreatorData == nullptr) {
    FRedwoodGetCharacterOutput Output;
    Output.Error = TEXT("Character creator data is required.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  Payload->SetStringField(TEXT("name"), Name);

  Payload->SetObjectField(
    TEXT("characterCreatorData"), CharacterCreatorData->GetRootObject()
  );

  Realm->Emit(
    TEXT("realm:characters:create"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      FRedwoodGetCharacterOutput Output;
      Output.Error = Error;

      const TSharedPtr<FJsonObject> *CharacterObj;
      if (MessageObject->TryGetObjectField(TEXT("character"), CharacterObj)) {
        Output.Character =
          URedwoodCommonGameSubsystem::ParseCharacter(*CharacterObj);
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::SetCharacterArchived(
  FString CharacterId, bool bArchived, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FString Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("id"), PlayerId);
  Payload->SetStringField(TEXT("characterId"), CharacterId);
  Payload->SetBoolField(TEXT("archived"), bArchived);

  Realm->Emit(
    TEXT("realm:characters:archive"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::GetCharacterData(
  FString CharacterIdOrName, FRedwoodGetCharacterOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodGetCharacterOutput Output;
    Output.Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("id"), PlayerId);
  Payload->SetStringField(TEXT("characterIdOrName"), CharacterIdOrName);

  Realm->Emit(
    TEXT("realm:characters:get"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      FRedwoodGetCharacterOutput Output;
      Output.Error = Error;

      const TSharedPtr<FJsonObject> *CharacterObj;
      if (MessageObject->TryGetObjectField(TEXT("character"), CharacterObj)) {
        Output.Character =
          URedwoodCommonGameSubsystem::ParseCharacter(*CharacterObj);
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::SetCharacterData(
  FString CharacterId,
  FString Name,
  USIOJsonObject *CharacterCreatorData,
  FRedwoodGetCharacterOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodGetCharacterOutput Output;
    Output.Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("characterId"), CharacterId);

  if (!Name.IsEmpty()) {
    Payload->SetStringField(TEXT("name"), Name);
  }

  if (IsValid(CharacterCreatorData)) {
    Payload->SetObjectField(
      TEXT("characterCreatorData"), CharacterCreatorData->GetRootObject()
    );
  }

  Realm->Emit(
    TEXT("realm:characters:set:client"),
    Payload,
    [this, OnOutput, CharacterId](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      FRedwoodGetCharacterOutput Output;
      Output.Error = Error;

      const TSharedPtr<FJsonObject> *CharacterObj;
      if (MessageObject->TryGetObjectField(TEXT("character"), CharacterObj)) {
        Output.Character =
          URedwoodCommonGameSubsystem::ParseCharacter(*CharacterObj);
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::SetSelectedCharacter(FString CharacterId) {
  SelectedCharacterId = CharacterId;

  if (!CurrentParty.bValid || !Realm.IsValid() || !Realm->bIsConnected) {
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("characterId"), SelectedCharacterId);

  Realm->Emit(TEXT("realm:parties:select-character"), Payload);
}

void URedwoodClientInterface::JoinMatchmaking(
  FString ProfileId,
  TArray<FString> InRegions,
  FRedwoodTicketingUpdateDelegate OnUpdate
) {
  if (SelectedCharacterId.IsEmpty()) {
    FRedwoodTicketingUpdate Update;
    Update.Type = ERedwoodTicketingUpdateType::JoinResponse;
    Update.Message =
      TEXT("Please select a character before joining matchmaking.");
    OnUpdate.ExecuteIfBound(Update);
    return;
  }

  TicketingProfileId = ProfileId;
  TicketingRegions = InRegions;
  OnTicketingUpdate = OnUpdate;
  AttemptJoinMatchmaking();
}

void URedwoodClientInterface::JoinQueue(
  FString ProxyId,
  FString ZoneName,
  bool bTransferWholeParty,
  FRedwoodTicketingUpdateDelegate OnUpdate
) {
  if (SelectedCharacterId.IsEmpty()) {
    FRedwoodTicketingUpdate Update;
    Update.Type = ERedwoodTicketingUpdateType::JoinResponse;
    Update.Message =
      TEXT("Please select a character before joining a server queue.");
    OnUpdate.ExecuteIfBound(Update);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("characterId"), SelectedCharacterId);

  TSharedPtr<FJsonObject> QueueData = MakeShareable(new FJsonObject);

  QueueData->SetStringField(TEXT("proxyId"), ProxyId);
  QueueData->SetStringField(TEXT("zoneName"), ZoneName);
  QueueData->SetBoolField(TEXT("transferWholeParty"), bTransferWholeParty);

  TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());
  QueueData->SetField(TEXT("priorZoneName"), NullValue);
  QueueData->SetField(TEXT("shardName"), NullValue);

  Payload->SetObjectField(TEXT("data"), QueueData);

  OnTicketingUpdate = OnUpdate;

  Realm
    ->Emit(TEXT("realm:ticketing:join:queue"), Payload, [this](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      FRedwoodTicketingUpdate Update;
      Update.Type = ERedwoodTicketingUpdateType::JoinResponse;
      Update.Message = Error;
      OnTicketingUpdate.ExecuteIfBound(Update);

      if (!Error.IsEmpty()) {
        OnTicketingUpdate = FRedwoodTicketingUpdateDelegate();
      }
    });
}

void URedwoodClientInterface::JoinCustom(
  bool bTransferWholeParty,
  TArray<FString> InRegions,
  FRedwoodTicketingUpdateDelegate OnUpdate
) {
  if (SelectedCharacterId.IsEmpty()) {
    FRedwoodTicketingUpdate Update;
    Update.Type = ERedwoodTicketingUpdateType::JoinResponse;
    Update.Message = TEXT("Please select a character before joining.");
    OnUpdate.ExecuteIfBound(Update);
    return;
  }

  TicketingRegions = InRegions;
  OnTicketingUpdate = OnUpdate;
  bTicketingTransferWholeParty = bTransferWholeParty;
  AttemptJoinCustom();
}

void URedwoodClientInterface::AttemptJoinCustom() {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodTicketingUpdate Update;
    Update.Type = ERedwoodTicketingUpdateType::JoinResponse;
    Update.Message = TEXT("Not connected to Realm.");
    OnTicketingUpdate.ExecuteIfBound(Update);
    OnTicketingUpdate = FRedwoodTicketingUpdateDelegate();
    return;
  }

  if (SelectedCharacterId.IsEmpty()) {
    FRedwoodTicketingUpdate Update;
    Update.Type = ERedwoodTicketingUpdateType::JoinResponse;
    Update.Message = TEXT("Please select a character before joining.");
    OnTicketingUpdate.ExecuteIfBound(Update);
    return;
  }

  if (PingAverages.Num() != Regions.Num()) {
    // we haven't finished getting a single set of ping averages yet
    // let's delay sending our join request
    UE_LOG(
      LogRedwood,
      Log,
      TEXT(
        "Not enough ping averages. Num averages: %d, num Regions: %d. Will attempt again."
      ),
      PingAverages.Num(),
      Regions.Num()
    );

    TimerManager.SetTimer(
      PingTimer, this, &URedwoodClientInterface::AttemptJoinCustom, 2.f, false
    );
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("characterId"), SelectedCharacterId);
  Payload->SetBoolField(
    TEXT("transferWholeParty"), bTicketingTransferWholeParty
  );

  TArray<TSharedPtr<FJsonValue>> DesiredRegions;
  for (FString RegionName : TicketingRegions) {
    float *Ping = PingAverages.Find(RegionName);

    if (Ping == nullptr) {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT("Could not find ping for region %s."),
        *RegionName
      );
      continue;
    }

    TSharedPtr<FJsonObject> RegionObject = MakeShareable(new FJsonObject);
    RegionObject->SetStringField(TEXT("name"), RegionName);
    RegionObject->SetNumberField(TEXT("ping"), *Ping);

    TSharedPtr<FJsonValueObject> Value =
      MakeShareable(new FJsonValueObject(RegionObject));

    DesiredRegions.Add(Value);
  }
  Payload->SetArrayField(TEXT("regions"), DesiredRegions);

  Realm
    ->Emit(TEXT("realm:ticketing:join:custom"), Payload, [this](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      FRedwoodTicketingUpdate Update;
      Update.Type = ERedwoodTicketingUpdateType::JoinResponse;
      Update.Message = Error;
      OnTicketingUpdate.ExecuteIfBound(Update);

      if (!Error.IsEmpty()) {
        OnTicketingUpdate = FRedwoodTicketingUpdateDelegate();
      }
    });
}

void URedwoodClientInterface::LeaveTicketing(
  FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FString Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  Realm->Emit(
    TEXT("realm:ticketing:leave"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::ListProxies(
  TArray<FString> PrivateProxyReferences,
  FRedwoodListProxiesOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodListProxiesOutput Output;
    Output.Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  TArray<TSharedPtr<FJsonValue>> PrivateProxyReferencesArray;
  for (FString Reference : PrivateProxyReferences) {
    TSharedPtr<FJsonValueString> Value =
      MakeShareable(new FJsonValueString(Reference));
    PrivateProxyReferencesArray.Add(Value);
  }
  Payload->SetArrayField(
    TEXT("privateProxyReferences"), PrivateProxyReferencesArray
  );

  Realm->Emit(
    TEXT("realm:servers:list-proxies"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      TArray<TSharedPtr<FJsonValue>> Proxies =
        MessageObject->GetArrayField(TEXT("proxies"));

      TArray<FRedwoodGameServerProxy> ProxiesStruct;
      for (TSharedPtr<FJsonValue> Proxy : Proxies) {
        TSharedPtr<FJsonObject> ProxyData = Proxy->AsObject();
        ProxiesStruct.Add(
          URedwoodCommonGameSubsystem::ParseServerProxy(ProxyData)
        );
      }

      FRedwoodListProxiesOutput Output;
      Output.Error = Error;
      Output.Proxies = ProxiesStruct;
      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::CreateProxy(
  bool bJoinSession,
  FRedwoodCreateProxyInput Parameters,
  FRedwoodCreateProxyOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodCreateProxyOutput Output;
    Output.Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  if (bJoinSession) {
    if (SelectedCharacterId.IsEmpty()) {
      FRedwoodCreateProxyOutput Output;
      Output.Error =
        TEXT("Please select a character before joining a session.");
      OnOutput.ExecuteIfBound(Output);
      return;
    }

    Payload->SetStringField(TEXT("joinCharacterId"), SelectedCharacterId);
  } else {
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());
    Payload->SetField(TEXT("joinCharacterId"), NullValue);
  }

  Payload->SetStringField(TEXT("name"), Parameters.Name);

  Payload->SetStringField(TEXT("region"), Parameters.Region);

  Payload->SetStringField(TEXT("modeId"), Parameters.ModeId);

  if (!Parameters.MapId.IsEmpty()) {
    Payload->SetStringField(TEXT("mapId"), Parameters.MapId);
  } else {
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());
    Payload->SetField(TEXT("mapId"), NullValue);
  }

  Payload->SetBoolField(TEXT("public"), Parameters.bPublic);

  Payload->SetBoolField(
    TEXT("proxyEndsWhenCollectionEnds"), Parameters.bProxyEndsWhenCollectionEnds
  );

  Payload->SetBoolField(TEXT("continuousPlay"), Parameters.bContinuousPlay);

  if (!Parameters.Password.IsEmpty()) {
    Payload->SetStringField(TEXT("password"), Parameters.Password);
  } else {
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());
    Payload->SetField(TEXT("password"), NullValue);
  }

  if (!Parameters.ShortCode.IsEmpty()) {
    Payload->SetStringField(TEXT("shortCode"), Parameters.ShortCode);
  } else {
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());
    Payload->SetField(TEXT("shortCode"), NullValue);
  }

  if (Parameters.Data) {
    Payload->SetObjectField(TEXT("data"), Parameters.Data->GetRootObject());
  }

  Payload->SetBoolField(TEXT("startOnBoot"), Parameters.bStartOnBoot);

  Realm->Emit(
    TEXT("realm:servers:create-proxy"),
    Payload,
    [this, OnOutput](auto Response) {
      FRedwoodCreateProxyOutput Output;

      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      Output.Error = MessageObject->GetStringField(TEXT("error"));

      MessageObject->TryGetStringField(
        TEXT("proxyReference"), Output.ProxyReference
      );

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::JoinProxyWithSingleInstance(
  FString ProxyReference,
  FString Password,
  FRedwoodJoinServerOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodJoinServerOutput Output;
    Output.Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("id"), PlayerId);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  if (SelectedCharacterId.IsEmpty()) {
    FRedwoodJoinServerOutput Output;
    Output.Error = TEXT("Please select a character before joining a session.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  Payload->SetStringField(TEXT("characterId"), SelectedCharacterId);

  Payload->SetStringField(TEXT("proxyReference"), ProxyReference);

  if (!Password.IsEmpty()) {
    Payload->SetStringField(TEXT("password"), Password);
  } else {
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());
    Payload->SetField(TEXT("password"), NullValue);
  }

  Realm->Emit(
    TEXT("realm:servers:join-proxy"),
    Payload,
    [this, OnOutput](auto Response) {
      FRedwoodJoinServerOutput Output;

      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      Output.Error = MessageObject->GetStringField(TEXT("error"));

      MessageObject->TryGetStringField(
        TEXT("connectionUri"), Output.ConnectionUri
      );
      MessageObject->TryGetStringField(TEXT("token"), Output.Token);

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::StopProxy(
  FString ServerProxyId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FString Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("id"), PlayerId);

  Payload->SetStringField(TEXT("proxyId"), ServerProxyId);

  Realm->Emit(
    TEXT("realm:servers:stop-proxy"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      OnOutput.ExecuteIfBound(MessageObject->GetStringField(TEXT("error")));
    }
  );
}

void URedwoodClientInterface::AttemptJoinMatchmaking() {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodTicketingUpdate Update;
    Update.Type = ERedwoodTicketingUpdateType::JoinResponse;
    Update.Message = TEXT("Not connected to Realm.");
    OnTicketingUpdate.ExecuteIfBound(Update);
    OnTicketingUpdate = FRedwoodTicketingUpdateDelegate();
    return;
  }

  if (PingAverages.Num() != Regions.Num()) {
    // we haven't finished getting a single set of ping averages yet
    // let's delay sending our join request
    UE_LOG(
      LogRedwood,
      Log,
      TEXT(
        "Not enough ping averages. Num averages: %d, num Regions: %d. Will attempt again."
      ),
      PingAverages.Num(),
      Regions.Num()
    );

    TimerManager.SetTimer(
      PingTimer,
      this,
      &URedwoodClientInterface::AttemptJoinMatchmaking,
      2.f,
      false
    );
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("characterId"), SelectedCharacterId);

  TSharedPtr<FJsonObject> MatchmakingData = MakeShareable(new FJsonObject);

  MatchmakingData->SetStringField(TEXT("profileId"), TicketingProfileId);

  TArray<TSharedPtr<FJsonValue>> DesiredRegions;
  for (FString RegionName : TicketingRegions) {
    float *Ping = PingAverages.Find(RegionName);

    if (Ping == nullptr) {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT("Could not find ping for region %s."),
        *RegionName
      );
      continue;
    }

    TSharedPtr<FJsonObject> RegionObject = MakeShareable(new FJsonObject);
    RegionObject->SetStringField(TEXT("name"), RegionName);
    RegionObject->SetNumberField(TEXT("ping"), *Ping);

    TSharedPtr<FJsonValueObject> Value =
      MakeShareable(new FJsonValueObject(RegionObject));

    DesiredRegions.Add(Value);
  }
  MatchmakingData->SetArrayField(TEXT("regions"), DesiredRegions);

  Payload->SetObjectField(TEXT("data"), MatchmakingData);

  Realm->Emit(
    TEXT("realm:ticketing:join:matchmaking"),
    Payload,
    [this](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      FRedwoodTicketingUpdate Update;
      Update.Type = ERedwoodTicketingUpdateType::JoinResponse;
      Update.Message = Error;
      OnTicketingUpdate.ExecuteIfBound(Update);

      if (!Error.IsEmpty()) {
        OnTicketingUpdate = FRedwoodTicketingUpdateDelegate();
      }
    }
  );
}

void URedwoodClientInterface::GetOrCreateParty(
  bool bCreateIfNotInParty, FRedwoodGetPartyOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodGetPartyOutput Output;
    Output.Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  if (SelectedCharacterId == TEXT("")) {
    FRedwoodGetPartyOutput Output;
    Output.Error = TEXT("Please select a character before joining a party.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("characterId"), SelectedCharacterId);
  Payload->SetBoolField(TEXT("createIfNotInParty"), bCreateIfNotInParty);

  Realm->Emit(
    TEXT("realm:parties:get:frontend"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      FRedwoodGetPartyOutput Output;
      Output.Error = Error;

      if (Error.IsEmpty()) {
        const TSharedPtr<FJsonObject> *PartyObj;
        if (MessageObject->TryGetObjectField(TEXT("party"), PartyObj)) {
          Output.Party = URedwoodCommonGameSubsystem::ParseParty(*PartyObj);
          CurrentParty = Output.Party;
        }
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::LeaveParty(FRedwoodErrorOutputDelegate OnOutput) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    OnOutput.ExecuteIfBound(TEXT("Not connected to Realm."));
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  Realm->Emit(
    TEXT("realm:parties:leave"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      if (Error.IsEmpty()) {
        CurrentParty = FRedwoodParty();
      }

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::InviteToParty(
  FString TargetPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    OnOutput.ExecuteIfBound(TEXT("Not connected to Realm."));
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("targetPlayerId"), TargetPlayerId);

  Realm->Emit(
    TEXT("realm:parties:invites:initiate"),
    Payload,
    [OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      OnOutput.ExecuteIfBound(Error);
    }
  );
}

void URedwoodClientInterface::ListPartyInvites(
  FRedwoodListPartyInvitesOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodListPartyInvitesOutput Output;
    Output.Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  Realm->Emit(
    TEXT("realm:parties:invites:get"),
    Payload,
    [OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      FRedwoodListPartyInvitesOutput Output;
      Output.Error = Error;
      if (Error.IsEmpty()) {
        TArray<TSharedPtr<FJsonValue>> Invites =
          MessageObject->GetArrayField(TEXT("invites"));

        Output.Invites =
          URedwoodCommonGameSubsystem::ParsePartyInvites(Invites);
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::RespondToPartyInvite(
  FString PartyId, bool bAccept, FRedwoodGetPartyOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodGetPartyOutput Output;
    Output.Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  if (SelectedCharacterId == TEXT("")) {
    FRedwoodGetPartyOutput Output;
    Output.Error =
      TEXT("Please select a character before responding to a party invite.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("characterId"), SelectedCharacterId);
  Payload->SetStringField(TEXT("partyId"), PartyId);
  Payload->SetBoolField(TEXT("accept"), bAccept);

  Realm->Emit(
    TEXT("realm:parties:invites:respond"),
    Payload,
    [OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      FRedwoodGetPartyOutput Output;
      Output.Error = Error;

      if (Error.IsEmpty()) {
        const TSharedPtr<FJsonObject> *PartyObj;
        if (MessageObject->TryGetObjectField(TEXT("party"), PartyObj)) {
          Output.Party = URedwoodCommonGameSubsystem::ParseParty(*PartyObj);
        }
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::PromoteToPartyLeader(
  FString TargetPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    OnOutput.ExecuteIfBound(TEXT("Not connected to Realm."));
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("targetPlayerId"), TargetPlayerId);

  Realm
    ->Emit(TEXT("realm:parties:promote"), Payload, [OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      OnOutput.ExecuteIfBound(Error);
    });
}

void URedwoodClientInterface::KickFromParty(
  FString TargetPlayerId, FRedwoodErrorOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    OnOutput.ExecuteIfBound(TEXT("Not connected to Realm."));
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("targetPlayerId"), TargetPlayerId);

  Realm->Emit(TEXT("realm:parties:kick"), Payload, [OnOutput](auto Response) {
    TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
    FString Error = MessageObject->GetStringField(TEXT("error"));
    OnOutput.ExecuteIfBound(Error);
  });
}

void URedwoodClientInterface::SetPartyData(
  FString LootType,
  USIOJsonObject *PartyData,
  FRedwoodGetPartyOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodGetPartyOutput Output;
    Output.Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  if (!LootType.IsEmpty()) {
    Payload->SetStringField(TEXT("lootType"), LootType);
  }

  if (IsValid(PartyData)) {
    Payload->SetObjectField(TEXT("data"), PartyData->GetRootObject());
  }

  Realm->Emit(TEXT("realm:parties:set"), Payload, [OnOutput](auto Response) {
    TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
    FString Error = MessageObject->GetStringField(TEXT("error"));

    FRedwoodGetPartyOutput Output;
    Output.Error = Error;

    if (Error.IsEmpty()) {
      const TSharedPtr<FJsonObject> *PartyObj;
      if (MessageObject->TryGetObjectField(TEXT("party"), PartyObj)) {
        Output.Party = URedwoodCommonGameSubsystem::ParseParty(*PartyObj);
      }
    }

    OnOutput.ExecuteIfBound(Output);
  });
}

void URedwoodClientInterface::SendEmoteToParty(FString Emote) {
  if (!Realm.IsValid() || !Realm->bIsConnected || !CurrentParty.bValid) {
    UE_LOG(
      LogRedwood,
      Warning,
      TEXT(
        "Cannot send emote to party: not connected to Realm or not in a party."
      )
    );
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("emote"), Emote);

  Realm->Emit(TEXT("realm:parties:emote"), Payload);
}

FString URedwoodClientInterface::GetConnectionConsoleCommand() {
  if (ServerConnection.IsEmpty() || ServerToken.IsEmpty()) {
    UE_LOG(LogRedwood, Error, TEXT("Server connection or token is empty."));
    return "";
  }

  if (SelectedCharacterId.IsEmpty()) {
    UE_LOG(LogRedwood, Error, TEXT("Selected character ID is empty."));
    return "";
  }

  TMap<FString, FString> Options;
  Options.Add("RedwoodAuth", "1");
  Options.Add("CharacterId", SelectedCharacterId);
  Options.Add("PlayerId", PlayerId);
  Options.Add("Token", ServerToken);

  TArray<FString> JoinedOptions;
  for (const TPair<FString, FString> &Option : Options) {
    JoinedOptions.Add(Option.Key + "=" + Option.Value);
  }

  FString OptionsString =
    UKismetStringLibrary::JoinStringArray(JoinedOptions, "?");
  FString ConnectionString =
    TEXT("open ") + ServerConnection + "?" + OptionsString;

  return ConnectionString;
}

FURL URedwoodClientInterface::GetConnectionURL() {
  FURL URL;
  URL.Valid = 0;
  if (ServerConnection.IsEmpty() || ServerToken.IsEmpty()) {
    UE_LOG(LogRedwood, Error, TEXT("Server connection or token is empty."));
    return URL;
  }

  if (SelectedCharacterId.IsEmpty()) {
    UE_LOG(LogRedwood, Error, TEXT("Selected character ID is empty."));
    return URL;
  }

  FString Host;
  FString Port;
  ServerConnection.Split(":", &Host, &Port);

  URL.Protocol = TEXT("unreal");
  URL.Host = Host;
  URL.Port = FCString::Atoi(*Port);
  URL.Valid = 1;

  TMap<FString, FString> Options;
  Options.Add("RedwoodAuth", "1");
  Options.Add("CharacterId", SelectedCharacterId);
  Options.Add("PlayerId", PlayerId);
  Options.Add("Token", ServerToken);
  for (const TPair<FString, FString> &Option : Options) {
    URL.AddOption(*(Option.Key + "=" + Option.Value));
  }

  return URL;
}

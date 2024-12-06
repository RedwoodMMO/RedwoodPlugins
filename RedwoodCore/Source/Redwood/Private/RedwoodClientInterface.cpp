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
    ISocketIOClientModule::Get().ReleaseNativePointer(Director);
    Director = nullptr;
  }

  if (Realm.IsValid()) {
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

  Director->OnDisconnectedCallback =
    [Uri, this](const ESIOConnectionCloseReason Reason) {
      if (Reason == ESIOConnectionCloseReason::CLOSE_REASON_DROP) {
        UE_LOG(
          LogRedwood, Error, TEXT("Lost connection to Director at %s"), *Uri
        );
        OnDirectorConnectionLost.Broadcast();
      }
    };

  Director->OnConnectedCallback =
    [Uri,
     OnDirectorConnected,
     this](const FString &InSocketId, const FString &InSessionId) {
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
          TEXT("Reestablished connection to Director at %s"),
          *Uri
        );
        OnDirectorConnectionReestablished.Broadcast();
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

      if (Minimum > 0) {
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
  return !PlayerId.IsEmpty() && !AuthToken.IsEmpty();
}

FString URedwoodClientInterface::GetPlayerId() {
  return PlayerId;
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
  Payload->SetStringField(TEXT("secret"), PasswordOrToken);
  Payload->SetStringField(TEXT("provider"), Provider);

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

FString URedwoodClientInterface::GetNickname() {
  return Nickname;
}

void URedwoodClientInterface::CancelWaitingForAccountVerification() {
  if (OnAccountVerified.IsBound()) {
    OnAccountVerified.Unbind();
  }
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

      Realm = ISocketIOClientModule::Get().NewValidNativePointer();
      Realm->MaxReconnectionAttempts = 0; // don't reattempt a realm connection
      bSentRealmConnected = false;

      Realm->OnDisconnectedCallback = [InRealm, this](
                                        const ESIOConnectionCloseReason Reason
                                      ) {
        if (Reason == ESIOConnectionCloseReason::CLOSE_REASON_DROP) {
          UE_LOG(
            LogRedwood,
            Error,
            TEXT(
              "Lost connection to Realm at %s; you will need to reinitialize the connection to restart the handshake."
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
        if (!bSentRealmConnected) {
          UE_LOG(
            LogRedwood, Log, TEXT("Connected to Realm at %s"), *InRealm.Uri
          );
          bSentRealmConnected = true;
          FinalizeRealmHandshake(Token, OnRealmConnected);
        }
      };

      UE_LOG(LogRedwood, Log, TEXT("Connecting to Realm at %s"), *InRealm.Uri);
      Realm->Connect(*InRealm.Uri);
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
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  Realm->OnEvent(
    TEXT("realm:servers:connect"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      TSharedPtr<FJsonObject> MessageObject = Message->AsObject();

      ServerConnection = MessageObject->GetStringField(TEXT("connection"));
      ServerToken = MessageObject->GetStringField(TEXT("token"));

      FString ConsoleCommand = GetConnectionConsoleCommand();
      OnRequestToJoinServer.Broadcast(ConsoleCommand);
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );
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

      TArray<FRedwoodCharacterBackend> CharactersStruct;
      for (TSharedPtr<FJsonValue> Character : Characters) {
        TSharedPtr<FJsonObject> CharacterData = Character->AsObject();

        CharactersStruct.Add(
          URedwoodCommonGameSubsystem::ParseCharacter(CharacterData)
        );
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

void URedwoodClientInterface::GetCharacterData(
  FString CharacterId, FRedwoodGetCharacterOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodGetCharacterOutput Output;
    Output.Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("id"), PlayerId);
  Payload->SetStringField(TEXT("characterId"), CharacterId);

  Realm->Emit(
    TEXT("realm:characters:get"),
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
  FString ProxyId, FString ZoneName, FRedwoodTicketingUpdateDelegate OnUpdate
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

void URedwoodClientInterface::ListServers(
  TArray<FString> PrivateServerReferences,
  FRedwoodListServersOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodListServersOutput Output;
    Output.Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  TArray<TSharedPtr<FJsonValue>> PrivateServerReferencesArray;
  for (FString Reference : PrivateServerReferences) {
    TSharedPtr<FJsonValueString> Value =
      MakeShareable(new FJsonValueString(Reference));
    PrivateServerReferencesArray.Add(Value);
  }
  Payload->SetArrayField(
    TEXT("privateServerReferences"), PrivateServerReferencesArray
  );

  Realm->Emit(
    TEXT("realm:servers:list"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      TArray<TSharedPtr<FJsonValue>> Servers =
        MessageObject->GetArrayField(TEXT("servers"));

      TArray<FRedwoodGameServerProxy> ServersStruct;
      for (TSharedPtr<FJsonValue> Server : Servers) {
        TSharedPtr<FJsonObject> ServerData = Server->AsObject();
        ServersStruct.Add(
          URedwoodCommonGameSubsystem::ParseServerProxy(ServerData)
        );
      }

      FRedwoodListServersOutput Output;
      Output.Error = Error;
      Output.Servers = ServersStruct;
      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::CreateServer(
  bool bJoinSession,
  FRedwoodCreateServerInput Parameters,
  FRedwoodCreateServerOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodCreateServerOutput Output;
    Output.Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  if (bJoinSession) {
    if (SelectedCharacterId.IsEmpty()) {
      FRedwoodCreateServerOutput Output;
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
    TEXT("realm:servers:create"),
    Payload,
    [this, OnOutput](auto Response) {
      FRedwoodCreateServerOutput Output;

      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      Output.Error = MessageObject->GetStringField(TEXT("error"));

      MessageObject->TryGetStringField(
        TEXT("serverReference"), Output.ServerReference
      );

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodClientInterface::JoinServerInstance(
  FString ServerReference,
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

  Payload->SetStringField(TEXT("serverReference"), ServerReference);

  if (!Password.IsEmpty()) {
    Payload->SetStringField(TEXT("password"), Password);
  } else {
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());
    Payload->SetField(TEXT("password"), NullValue);
  }

  Realm->Emit(
    TEXT("realm:servers:join"),
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

void URedwoodClientInterface::StopServer(
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
    TEXT("realm:servers:stop"),
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
    }
  );
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
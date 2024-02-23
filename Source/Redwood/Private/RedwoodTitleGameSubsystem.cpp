// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodTitleGameSubsystem.h"
#include "RedwoodGameplayTags.h"
#include "RedwoodSettings.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/KismetStringLibrary.h"
#include "LatencyCheckerLibrary.h"
#include "SocketIOClient.h"

void URedwoodTitleGameSubsystem::Initialize(FSubsystemCollectionBase &Collection
) {
  Super::Initialize(Collection);
}

void URedwoodTitleGameSubsystem::Deinitialize() {
  Super::Deinitialize();

  if (Director.IsValid()) {
    ISocketIOClientModule::Get().ReleaseNativePointer(Director);
    Director = nullptr;
  }

  if (Realm.IsValid()) {
    ISocketIOClientModule::Get().ReleaseNativePointer(Realm);
    Realm = nullptr;
  }
}

void URedwoodTitleGameSubsystem::InitializeDirectorConnection(
  FRedwoodOnSocketConnected OnDirectorConnected
) {
  Director = ISocketIOClientModule::Get().NewValidNativePointer();

  Director->OnEvent(
    TEXT("realm:regions"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      HandleRegionsChanged(Event, Message);
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

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

  Director->OnEvent(
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

  Director->OnEvent(
    TEXT("realm:ticketing:ready"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      TSharedPtr<FJsonObject> MessageObject = Message->AsObject();

      TicketingConnection = MessageObject->GetStringField(TEXT("connection"));
      TicketingToken = MessageObject->GetStringField(TEXT("token"));

      FRedwoodTicketingUpdate Update;
      Update.Type = ERedwoodTicketingUpdateType::Ready;
      Update.Message = TEXT("The match is ready.");

      OnTicketingUpdate.ExecuteIfBound(Update);
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  Director->OnEvent(
    TEXT("realm:ticketing:ticket-stale"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      FRedwoodTicketingUpdate Update;
      Update.Type = ERedwoodTicketingUpdateType::TicketStale;
      Update.Message = TEXT("Could not find a match. Please try again.");
      OnTicketingUpdate.ExecuteIfBound(Update);
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  Director->OnConnectedCallback = [OnDirectorConnected](
                                    const FString &InSocketId,
                                    const FString &InSessionId
                                  ) {
    FRedwoodSocketConnected Result;
    Result.Error = TEXT("");
    Result.SocketId = InSocketId;
    Result.SessionId = InSessionId;
    OnDirectorConnected.ExecuteIfBound(Result);
  };

  URedwoodSettings *RedwoodSettings = GetMutableDefault<URedwoodSettings>();
  Director->Connect(*RedwoodSettings->DirectorUri);
}

void URedwoodTitleGameSubsystem::HandleRegionsChanged(
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

  GetWorld()->GetTimerManager().ClearTimer(PingTimer);
  InitiatePings();
}

void URedwoodTitleGameSubsystem::InitiatePings() {
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
      PingAverages.Add(Itr.Key, Minimum);

      OnPingResultReceived.Broadcast(
        Itr.Key, FMath::FloorToInt(Minimum * 1000)
      );
    }

    // queue the next set of pings
    PingAttemptsLeft = bHasWebsocketRegion ? 1 : PingAttempts;

    GetWorld()->GetTimerManager().SetTimer(
      PingTimer,
      this,
      &URedwoodTitleGameSubsystem::InitiatePings,
      RedwoodSettings->PingFrequency,
      false
    );
  }
}

void URedwoodTitleGameSubsystem::HandlePingResult(
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

void URedwoodTitleGameSubsystem::Register(
  const FString &Username,
  const FString &Password,
  FRedwoodOnAuthUpdate OnUpdate
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

void URedwoodTitleGameSubsystem::Login(
  const FString &Username,
  const FString &PasswordOrToken,
  FRedwoodOnAuthUpdate OnUpdate
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

  Director->Emit(
    TEXT("player:login:username"),
    Payload,
    [this, OnUpdate](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      PlayerId = MessageObject->GetStringField(TEXT("playerId"));
      AuthToken = MessageObject->GetStringField(TEXT("token"));

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

void URedwoodTitleGameSubsystem::CancelWaitingForAccountVerification() {
  if (OnAccountVerified.IsBound()) {
    OnAccountVerified.Unbind();
  }
}

void URedwoodTitleGameSubsystem::ListRealms(FRedwoodOnListRealms OnResult) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodRealmsResult Result;
    Result.Error = TEXT("Not connected to Director.");
    OnResult.ExecuteIfBound(Result);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  Director->Emit(TEXT("realm:list"), Payload, [this, OnResult](auto Response) {
    TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

    FRedwoodRealmsResult Result;

    Result.Error = MessageObject->GetStringField(TEXT("error"));
    Result.bSingleRealm = MessageObject->GetBoolField(TEXT("isSingleRealm"));

    const TArray<TSharedPtr<FJsonValue>> &Realms =
      MessageObject->GetArrayField(TEXT("realms"));

    for (TSharedPtr<FJsonValue> InRealm : Realms) {
      FRedwoodRealm OutRealm;
      OutRealm.Id = InRealm->AsObject()->GetStringField(TEXT("id"));
      OutRealm.Name = InRealm->AsObject()->GetStringField(TEXT("name"));
      OutRealm.Uri = InRealm->AsObject()->GetStringField(TEXT("uri"));
      OutRealm.PingHost = InRealm->AsObject()->GetStringField(TEXT("pingHost"));

      Result.Realms.Add(OutRealm);
    }

    OnResult.ExecuteIfBound(Result);
  });
}

void URedwoodTitleGameSubsystem::InitializeSingleRealmConnection(
  FRedwoodOnSocketConnected OnRealmConnected
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodSocketConnected Result;
    Result.Error = TEXT("Not connected to Director.");
    OnRealmConnected.ExecuteIfBound(Result);
    return;
  }

  Realm = Director;

  FRedwoodSocketConnected Result;
  Result.Error = TEXT("");
  Result.SocketId = Realm->SocketId;
  Result.SessionId = Realm->SessionId;
  OnRealmConnected.ExecuteIfBound(Result);
}

void URedwoodTitleGameSubsystem::InitializeRealmConnection(
  FRedwoodRealm InRealm, FRedwoodOnSocketConnected OnRealmConnected
) {
  Realm = ISocketIOClientModule::Get().NewValidNativePointer();

  Realm->OnConnectedCallback =
    [OnRealmConnected](const FString &InSocketId, const FString &InSessionId) {
      FRedwoodSocketConnected Result;
      Result.Error = TEXT("");
      Result.SocketId = InSocketId;
      Result.SessionId = InSessionId;
      OnRealmConnected.ExecuteIfBound(Result);
    };

  Realm->Connect(*InRealm.Uri);
}

void URedwoodTitleGameSubsystem::ListCharacters(
  FRedwoodOnListCharacters OnResult
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodCharactersResult Result;
    Result.Error = TEXT("Not connected to Realm.");
    OnResult.ExecuteIfBound(Result);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  Realm->Emit(
    TEXT("realm:characters:list"),
    Payload,
    [this, OnResult](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      TArray<TSharedPtr<FJsonValue>> Characters =
        MessageObject->GetArrayField(TEXT("characters"));

      TArray<FRedwoodPlayerCharacter> CharactersStruct;
      for (TSharedPtr<FJsonValue> Character : Characters) {
        TSharedPtr<FJsonObject> CharacterData = Character->AsObject();
        FRedwoodPlayerCharacter CharacterStruct;

        CharacterStruct.Id = CharacterData->GetStringField(TEXT("id"));
        USIOJsonObject *CharacterDataObject = NewObject<USIOJsonObject>();
        CharacterDataObject->SetRootObject(
          CharacterData->GetObjectField(TEXT("data"))
        );
        CharacterStruct.Data = CharacterDataObject;

        CharactersStruct.Add(CharacterStruct);
      }

      FRedwoodCharactersResult Result;
      Result.Error = Error;
      Result.Characters = CharactersStruct;
      OnResult.ExecuteIfBound(Result);
    }
  );
}

void URedwoodTitleGameSubsystem::CreateCharacter(
  USIOJsonObject *Data, FRedwoodOnGetCharacter OnResult
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodCharacterResult Result;
    Result.Error = TEXT("Not connected to Realm.");
    OnResult.ExecuteIfBound(Result);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetObjectField(TEXT("data"), Data->GetRootObject());

  Realm->Emit(
    TEXT("realm:characters:create"),
    Payload,
    [this, OnResult](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      TSharedPtr<FJsonObject> CharacterObj =
        MessageObject->GetObjectField(TEXT("character"));

      FString CharacterId = CharacterObj->GetStringField(TEXT("id"));
      TSharedPtr<FJsonObject> CharacterData =
        CharacterObj->GetObjectField(TEXT("data"));

      FRedwoodPlayerCharacter Character;
      Character.Id = CharacterId;
      Character.Data = NewObject<USIOJsonObject>();
      Character.Data->SetRootObject(CharacterData);

      FRedwoodCharacterResult Result;
      Result.Error = Error;
      Result.Character = Character;
      OnResult.ExecuteIfBound(Result);
    }
  );
}

void URedwoodTitleGameSubsystem::GetCharacterData(
  FString CharacterId, FRedwoodOnGetCharacter OnResult
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodCharacterResult Result;
    Result.Error = TEXT("Not connected to Realm.");
    OnResult.ExecuteIfBound(Result);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("characterId"), CharacterId);

  Realm->Emit(
    TEXT("realm:characters:get"),
    Payload,
    [this, OnResult, CharacterId](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      TSharedPtr<FJsonObject> CharacterObj =
        MessageObject->GetObjectField(TEXT("character"));

      TSharedPtr<FJsonObject> CharacterData =
        CharacterObj->GetObjectField(TEXT("data"));

      USIOJsonObject *CharacterDataObject = NewObject<USIOJsonObject>();
      CharacterDataObject->SetRootObject(CharacterData);

      FRedwoodPlayerCharacter Character;
      Character.Id = CharacterId;
      Character.Data = CharacterDataObject;

      FRedwoodCharacterResult Result;
      Result.Error = Error;
      Result.Character = Character;
      OnResult.ExecuteIfBound(Result);
    }
  );
}

void URedwoodTitleGameSubsystem::SetCharacterData(
  FString CharacterId, USIOJsonObject *Data, FRedwoodOnGetCharacter OnResult
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodCharacterResult Result;
    Result.Error = TEXT("Not connected to Realm.");
    OnResult.ExecuteIfBound(Result);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("characterId"), CharacterId);
  Payload->SetObjectField(TEXT("data"), Data->GetRootObject());

  Realm->Emit(
    TEXT("realm:characters:set"),
    Payload,
    [this, OnResult, CharacterId](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      TSharedPtr<FJsonObject> CharacterObj =
        MessageObject->GetObjectField(TEXT("character"));

      TSharedPtr<FJsonObject> CharacterData =
        CharacterObj->GetObjectField(TEXT("data"));

      USIOJsonObject *CharacterDataObject = NewObject<USIOJsonObject>();
      CharacterDataObject->SetRootObject(CharacterData);

      FRedwoodPlayerCharacter Character;
      Character.Id = CharacterId;
      Character.Data = CharacterDataObject;

      FRedwoodCharacterResult Result;
      Result.Error = Error;
      Result.Character = Character;
      OnResult.ExecuteIfBound(Result);
    }
  );
}

void URedwoodTitleGameSubsystem::JoinTicketing(
  FString Profile, FRedwoodOnTicketingUpdate OnUpdate
) {
  TicketingProfile = Profile;
  OnTicketingUpdate = OnUpdate;
  AttemptJoinTicketing();
}

FRedwoodGameServerProxy URedwoodTitleGameSubsystem::ParseServerProxy(
  TSharedPtr<FJsonObject> ServerProxy
) {
  FRedwoodGameServerProxy Server;

  Server.Id = ServerProxy->GetStringField(TEXT("id"));

  FDateTime::ParseIso8601(
    *ServerProxy->GetStringField(TEXT("createdAt")), Server.CreatedAt
  );

  FDateTime::ParseIso8601(
    *ServerProxy->GetStringField(TEXT("updatedAt")), Server.UpdatedAt
  );

  FDateTime::ParseIso8601(
    *ServerProxy->GetStringField(TEXT("endedAt")), Server.EndedAt
  );

  Server.Name = ServerProxy->GetStringField(TEXT("name"));

  Server.Region = ServerProxy->GetStringField(TEXT("region"));

  Server.Mode = ServerProxy->GetStringField(TEXT("mode"));

  Server.bPublic = ServerProxy->GetBoolField(TEXT("public"));

  Server.bContinuousPlay = ServerProxy->GetBoolField(TEXT("continuousPlay"));

  ServerProxy->TryGetBoolField(TEXT("hasPassword"), Server.bHasPassword);

  ServerProxy->TryGetStringField(TEXT("password"), Server.Password);

  Server.ShortCode = ServerProxy->GetStringField(TEXT("shortCode"));

  Server.CurrentPlayers = ServerProxy->GetIntegerField(TEXT("currentPlayers"));

  Server.MaxPlayers = ServerProxy->GetIntegerField(TEXT("maxPlayers"));

  TSharedPtr<FJsonObject> Data = ServerProxy->GetObjectField(TEXT("data"));
  USIOJsonObject *DataObject = NewObject<USIOJsonObject>();
  DataObject->SetRootObject(Data);
  Server.Data = DataObject;

  Server.OwnerPlayerIdentityId =
    ServerProxy->GetStringField(TEXT("ownerPlayerIdentityId"));

  Server.ActiveInstanceId =
    ServerProxy->GetStringField(TEXT("activeInstanceId"));

  return Server;
}

FRedwoodGameServerInstance URedwoodTitleGameSubsystem::ParseServerInstance(
  TSharedPtr<FJsonObject> ServerInstance
) {
  FRedwoodGameServerInstance Instance;

  Instance.Id = ServerInstance->GetStringField(TEXT("id"));

  FDateTime::ParseIso8601(
    *ServerInstance->GetStringField(TEXT("createdAt")), Instance.CreatedAt
  );

  FDateTime::ParseIso8601(
    *ServerInstance->GetStringField(TEXT("updatedAt")), Instance.UpdatedAt
  );

  Instance.ProviderId = ServerInstance->GetStringField(TEXT("providerId"));

  FDateTime::ParseIso8601(
    *ServerInstance->GetStringField(TEXT("startedAt")), Instance.StartedAt
  );

  FDateTime::ParseIso8601(
    *ServerInstance->GetStringField(TEXT("endedAt")), Instance.EndedAt
  );

  Instance.Connection = ServerInstance->GetStringField(TEXT("connection"));

  Instance.ContainerId = ServerInstance->GetStringField(TEXT("containerId"));

  Instance.ProxyId = ServerInstance->GetStringField(TEXT("proxyId"));

  return Instance;
}

void URedwoodTitleGameSubsystem::ListServers(
  TArray<FString> PrivateServerReferences, FRedwoodOnListServers OnResult
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodListServers Result;
    Result.Error = TEXT("Not connected to Realm.");
    OnResult.ExecuteIfBound(Result);
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
    [this, OnResult](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      TArray<TSharedPtr<FJsonValue>> Servers =
        MessageObject->GetArrayField(TEXT("servers"));

      TArray<FRedwoodGameServerProxy> ServersStruct;
      for (TSharedPtr<FJsonValue> Server : Servers) {
        TSharedPtr<FJsonObject> ServerData = Server->AsObject();
        ServersStruct.Add(
          URedwoodTitleGameSubsystem::ParseServerProxy(ServerData)
        );
      }

      FRedwoodListServers Result;
      Result.Error = Error;
      Result.Servers = ServersStruct;
      OnResult.ExecuteIfBound(Result);
    }
  );
}

void URedwoodTitleGameSubsystem::CreateServer(
  FRedwoodCreateServerParameters Parameters, FRedwoodOnGetServer OnResult
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodGetServer Result;
    Result.Error = TEXT("Not connected to Realm.");
    OnResult.ExecuteIfBound(Result);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  Payload->SetStringField(TEXT("name"), Parameters.Name);

  Payload->SetStringField(TEXT("region"), Parameters.Region);

  Payload->SetStringField(TEXT("mode"), Parameters.Mode);

  Payload->SetBoolField(TEXT("public"), Parameters.bPublic);

  Payload->SetBoolField(TEXT("continuousPlay"), Parameters.bContinuousPlay);

  if (!Parameters.Password.IsEmpty()) {
    Payload->SetStringField(TEXT("password"), Parameters.Password);
  } else {
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());
    Payload->SetField(TEXT("password"), NullValue);
  }

  Payload->SetStringField(TEXT("shortCode"), Parameters.ShortCode);

  Payload->SetNumberField(TEXT("maxPlayers"), Parameters.MaxPlayers);

  if (Parameters.Data) {
    Payload->SetObjectField(TEXT("data"), Parameters.Data->GetRootObject());
  }

  Realm->Emit(
    TEXT("realm:servers:create"),
    Payload,
    [this, OnResult](auto Response) {
      FRedwoodGetServer Result;

      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      Result.Error = MessageObject->GetStringField(TEXT("error"));

      const TSharedPtr<FJsonObject> *ServerInstanceObject = nullptr;
      if (MessageObject->TryGetObjectField(
            TEXT("instance"), ServerInstanceObject
          )) {
        FRedwoodGameServerInstance ServerInstance =
          URedwoodTitleGameSubsystem::ParseServerInstance(*ServerInstanceObject
          );
        Result.ServerInstance = ServerInstance;
      }

      OnResult.ExecuteIfBound(Result);
    }
  );
}

void URedwoodTitleGameSubsystem::GetServerInstance(
  FString ServerReference, FString Password, FRedwoodOnGetServer OnResult
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodGetServer Result;
    Result.Error = TEXT("Not connected to Realm.");
    OnResult.ExecuteIfBound(Result);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("id"), PlayerId);

  Payload->SetStringField(TEXT("serverReference"), ServerReference);

  if (!Password.IsEmpty()) {
    Payload->SetStringField(TEXT("password"), Password);
  } else {
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());
    Payload->SetField(TEXT("password"), NullValue);
  }

  Realm
    ->Emit(TEXT("realm:servers:get"), Payload, [this, OnResult](auto Response) {
      FRedwoodGetServer Result;

      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      Result.Error = MessageObject->GetStringField(TEXT("error"));

      const TSharedPtr<FJsonObject> *ServerInstanceObject = nullptr;
      if (MessageObject->TryGetObjectField(
            TEXT("instance"), ServerInstanceObject
          )) {
        FRedwoodGameServerInstance ServerInstance =
          URedwoodTitleGameSubsystem::ParseServerInstance(*ServerInstanceObject
          );
        Result.ServerInstance = ServerInstance;
      }

      OnResult.ExecuteIfBound(Result);
    });
}

void URedwoodTitleGameSubsystem::StopServer(
  FString ServerProxyId, FRedwoodOnSimpleResult OnResult
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FString Error = TEXT("Not connected to Realm.");
    OnResult.ExecuteIfBound(Error);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("id"), PlayerId);

  Payload->SetStringField(TEXT("proxyId"), ServerProxyId);

  Realm->Emit(
    TEXT("realm:servers:stop"),
    Payload,
    [this, OnResult](auto Response) {
      FRedwoodGetServer Result;

      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();

      OnResult.ExecuteIfBound(MessageObject->GetStringField(TEXT("error")));
    }
  );
}

void URedwoodTitleGameSubsystem::AttemptJoinTicketing() {
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
    GetWorld()->GetTimerManager().SetTimer(
      PingTimer,
      this,
      &URedwoodTitleGameSubsystem::AttemptJoinTicketing,
      2.f,
      false
    );
    return;
  }

  TArray<FRedwoodRegionLatencySort> UnsortedRegions;
  TArray<FRedwoodRegionLatencySort> SortedRegions;

  for (auto Itr : Regions) {
    FRedwoodRegionLatencySort Region;
    Region.Id = Itr.Key;
    Region.RTT = PingAverages[Itr.Key];
    UnsortedRegions.Add(Region);
  }

  while (UnsortedRegions.Num() > 0) {
    int32 LowestIndex = 0;
    for (int32 i = 0; i < UnsortedRegions.Num(); i++) {
      if (UnsortedRegions[i].RTT < UnsortedRegions[LowestIndex].RTT) {
        LowestIndex = i;
      }
    }

    SortedRegions.Add(UnsortedRegions[LowestIndex]);
    UnsortedRegions.RemoveAt(LowestIndex);
  }

  // TODO: Support multiple profiles
  TArray<TSharedPtr<FJsonValue>> DesiredProfiles;
  TSharedPtr<FJsonValueString> TicketingProfileValue =
    MakeShareable(new FJsonValueString(TicketingProfile));
  DesiredProfiles.Add(TicketingProfileValue);

  TArray<TSharedPtr<FJsonValue>> DesiredRegions;
  for (FRedwoodRegionLatencySort Region : SortedRegions) {
    TSharedPtr<FJsonValueString> Value =
      MakeShareable(new FJsonValueString(Region.Id));
    DesiredRegions.Add(Value);
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetArrayField(TEXT("profiles"), DesiredProfiles);
  Payload->SetArrayField(TEXT("regions"), DesiredRegions);

  Realm->Emit(TEXT("realm:ticketing:join"), Payload, [this](auto Response) {
    FString Error = Response[0]->AsString();
    FRedwoodTicketingUpdate Update;
    Update.Type = ERedwoodTicketingUpdateType::JoinResponse;
    Update.Message = Error;
    OnTicketingUpdate.ExecuteIfBound(Update);
  });
}

FString URedwoodTitleGameSubsystem::GetConnectionString(FString CharacterId) {
  if (TicketingConnection.IsEmpty() || TicketingToken.IsEmpty()) {
    UE_LOG(
      LogRedwood,
      Error,
      TEXT(
        "Ticketing connection or token is empty. Did you call JoinTicketing and got an update that was of type Ready?"
      )
    );
    return "";
  }

  TMap<FString, FString> Options;
  Options.Add("RedwoodAuth", "1");
  Options.Add("PlayerId", PlayerId);
  Options.Add("CharacterId", CharacterId);
  Options.Add("Token", TicketingToken);

  TArray<FString> JoinedOptions;
  for (const TPair<FString, FString> &Option : Options) {
    JoinedOptions.Add(Option.Key + "=" + Option.Value);
  }

  FString OptionsString =
    UKismetStringLibrary::JoinStringArray(JoinedOptions, "?");
  FString ConnectionString = TicketingConnection + "?" + OptionsString;

  return ConnectionString;
}
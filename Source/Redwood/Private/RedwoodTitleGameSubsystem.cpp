// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodTitleGameSubsystem.h"
#include "RedwoodGameplayTags.h"
#include "RedwoodSettings.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/KismetStringLibrary.h"
#include "LatencyCheckerLibrary.h"
#include "SocketIOClient.h"

#include "JsonModern.h"

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
  FRedwoodSocketConnectedDelegate OnDirectorConnected
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

  Director->OnEvent(
    TEXT("realm:servers:connect"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      TSharedPtr<FJsonObject> MessageObject = Message->AsObject();

      ServerConnection = MessageObject->GetStringField(TEXT("connection"));
      ServerToken = MessageObject->GetStringField(TEXT("token"));

      URedwoodSettings *RedwoodSettings = GetMutableDefault<URedwoodSettings>();
      if (RedwoodSettings->bAutoConnectToServers) {
        FString ConsoleCommand = GetConnectionConsoleCommand();
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), ConsoleCommand);
      } else {
        OnRequestToJoinServer.Broadcast();
      }
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  Director->OnConnectedCallback = [OnDirectorConnected](
                                    const FString &InSocketId,
                                    const FString &InSessionId
                                  ) {
    FRedwoodSocketConnected Details;
    Details.Error = TEXT("");
    Details.SocketId = InSocketId;
    Details.SessionId = InSessionId;
    OnDirectorConnected.ExecuteIfBound(Details);
  };

  URedwoodSettings *RedwoodSettings = GetMutableDefault<URedwoodSettings>();
  Director->Connect(*RedwoodSettings->DirectorUri);
}

bool URedwoodTitleGameSubsystem::IsDirectorConnected() {
  return Director.IsValid() && Director->bIsConnected;
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
    }

    OnPingsReceived.Broadcast();

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

void URedwoodTitleGameSubsystem::Logout() {
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
  }
}

bool URedwoodTitleGameSubsystem::IsLoggedIn() {
  return !PlayerId.IsEmpty() && !AuthToken.IsEmpty();
}

void URedwoodTitleGameSubsystem::Login(
  const FString &Username,
  const FString &PasswordOrToken,
  FRedwoodAuthUpdateDelegate OnUpdate
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodAuthUpdate Update;
    Update.Type = ERedwoodAuthUpdateType::Error;
    Update.Message = TEXT("Not connected to Director.");
    OnUpdate.ExecuteIfBound(Update);
    return;
  }

  JsonModern::FJson Payload = {
    {"username", Username}, {"secret", PasswordOrToken}};

  Director->Emit(
    TEXT("player:login:username"),
    JsonModern::ToUnreal(Payload),
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

void URedwoodTitleGameSubsystem::ListRealms(
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
    Output.bSingleRealm = MessageObject->GetBoolField(TEXT("isSingleRealm"));

    const TArray<TSharedPtr<FJsonValue>> &Realms =
      MessageObject->GetArrayField(TEXT("realms"));

    for (TSharedPtr<FJsonValue> InRealm : Realms) {
      FRedwoodRealm OutRealm;
      OutRealm.Id = InRealm->AsObject()->GetStringField(TEXT("id"));
      OutRealm.Name = InRealm->AsObject()->GetStringField(TEXT("name"));
      OutRealm.Uri = InRealm->AsObject()->GetStringField(TEXT("uri"));
      OutRealm.PingHost = InRealm->AsObject()->GetStringField(TEXT("pingHost"));

      Output.Realms.Add(OutRealm);
    }

    OnOutput.ExecuteIfBound(Output);
  });
}

void URedwoodTitleGameSubsystem::InitializeSingleRealmConnection(
  FRedwoodSocketConnectedDelegate OnRealmConnected
) {
  if (!Director.IsValid() || !Director->bIsConnected) {
    FRedwoodSocketConnected Output;
    Output.Error = TEXT("Not connected to Director.");
    OnRealmConnected.ExecuteIfBound(Output);
    return;
  }

  Realm = Director;

  FRedwoodSocketConnected Output;
  Output.Error = TEXT("");
  Output.SocketId = Realm->SocketId;
  Output.SessionId = Realm->SessionId;
  OnRealmConnected.ExecuteIfBound(Output);
}

void URedwoodTitleGameSubsystem::InitializeRealmConnection(
  FRedwoodRealm InRealm, FRedwoodSocketConnectedDelegate OnRealmConnected
) {
  Realm = ISocketIOClientModule::Get().NewValidNativePointer();

  Realm->OnConnectedCallback =
    [OnRealmConnected](const FString &InSocketId, const FString &InSessionId) {
      FRedwoodSocketConnected Output;
      Output.Error = TEXT("");
      Output.SocketId = InSocketId;
      Output.SessionId = InSessionId;
      OnRealmConnected.ExecuteIfBound(Output);
    };

  Realm->Connect(*InRealm.Uri);
}

bool URedwoodTitleGameSubsystem::IsRealmConnected() {
  return Realm.IsValid() && Realm->bIsConnected;
}

TMap<FString, float> URedwoodTitleGameSubsystem::GetRegions() {
  return PingAverages;
}

void URedwoodTitleGameSubsystem::ListCharacters(
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

      TArray<FRedwoodCharacter> CharactersStruct;
      for (TSharedPtr<FJsonValue> Character : Characters) {
        TSharedPtr<FJsonObject> CharacterData = Character->AsObject();
        FRedwoodCharacter CharacterStruct;

        CharacterStruct.Id = CharacterData->GetStringField(TEXT("id"));
        USIOJsonObject *CharacterDataObject = NewObject<USIOJsonObject>();
        CharacterDataObject->SetRootObject(
          CharacterData->GetObjectField(TEXT("data"))
        );
        CharacterStruct.Data = CharacterDataObject;

        CharactersStruct.Add(CharacterStruct);
      }

      FRedwoodListCharactersOutput Output;
      Output.Error = Error;
      Output.Characters = CharactersStruct;
      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodTitleGameSubsystem::CreateCharacter(
  USIOJsonObject *Metadata,
  USIOJsonObject *EquippedInventory,
  USIOJsonObject *NonequippedInventory,
  USIOJsonObject *Data,
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

  if (IsValid(Metadata)) {
    Payload->SetObjectField(TEXT("metadata"), Metadata->GetRootObject());
  } else {
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());
    Payload->SetField(TEXT("metadata"), NullValue);
  }

  if (IsValid(EquippedInventory)) {
    Payload->SetObjectField(
      TEXT("equippedInventory"), EquippedInventory->GetRootObject()
    );
  } else {
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());
    Payload->SetField(TEXT("equippedInventory"), NullValue);
  }

  if (IsValid(NonequippedInventory)) {
    Payload->SetObjectField(
      TEXT("nonequippedInventory"), NonequippedInventory->GetRootObject()
    );
  } else {
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());
    Payload->SetField(TEXT("nonequippedInventory"), NullValue);
  }

  if (IsValid(Data)) {
    Payload->SetObjectField(TEXT("data"), Data->GetRootObject());
  } else {
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());
    Payload->SetField(TEXT("data"), NullValue);
  }

  Realm->Emit(
    TEXT("realm:characters:create"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      TSharedPtr<FJsonObject> CharacterObj =
        MessageObject->GetObjectField(TEXT("character"));

      FString CharacterId = CharacterObj->GetStringField(TEXT("id"));
      TSharedPtr<FJsonObject> CharacterData =
        CharacterObj->GetObjectField(TEXT("data"));

      FRedwoodCharacter Character;
      Character.Id = CharacterId;
      Character.Data = NewObject<USIOJsonObject>();
      Character.Data->SetRootObject(CharacterData);

      FRedwoodGetCharacterOutput Output;
      Output.Error = Error;
      Output.Character = Character;
      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodTitleGameSubsystem::GetCharacterData(
  FString CharacterId, FRedwoodGetCharacterOutputDelegate OnOutput
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

  Realm->Emit(
    TEXT("realm:characters:get"),
    Payload,
    [this, OnOutput, CharacterId](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      TSharedPtr<FJsonObject> CharacterObj =
        MessageObject->GetObjectField(TEXT("character"));

      TSharedPtr<FJsonObject> CharacterData =
        CharacterObj->GetObjectField(TEXT("data"));

      USIOJsonObject *CharacterDataObject = NewObject<USIOJsonObject>();
      CharacterDataObject->SetRootObject(CharacterData);

      FRedwoodCharacter Character;
      Character.Id = CharacterId;
      Character.Data = CharacterDataObject;

      FRedwoodGetCharacterOutput Output;
      Output.Error = Error;
      Output.Character = Character;
      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodTitleGameSubsystem::SetCharacterData(
  FString CharacterId,
  USIOJsonObject *Metadata,
  USIOJsonObject *EquippedInventory,
  USIOJsonObject *NonequippedInventory,
  USIOJsonObject *Data,
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

  if (IsValid(Metadata)) {
    Payload->SetObjectField(TEXT("metadata"), Metadata->GetRootObject());
  }

  if (IsValid(EquippedInventory)) {
    Payload->SetObjectField(
      TEXT("equippedInventory"), EquippedInventory->GetRootObject()
    );
  }

  if (IsValid(NonequippedInventory)) {
    Payload->SetObjectField(
      TEXT("nonequippedInventory"), NonequippedInventory->GetRootObject()
    );
  }

  if (IsValid(Data)) {
    Payload->SetObjectField(TEXT("data"), Data->GetRootObject());
  }

  Realm->Emit(
    TEXT("realm:characters:set"),
    Payload,
    [this, OnOutput, CharacterId](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      TSharedPtr<FJsonObject> CharacterObj =
        MessageObject->GetObjectField(TEXT("character"));

      TSharedPtr<FJsonObject> CharacterData =
        CharacterObj->GetObjectField(TEXT("data"));

      USIOJsonObject *CharacterDataObject = NewObject<USIOJsonObject>();
      CharacterDataObject->SetRootObject(CharacterData);

      FRedwoodCharacter Character;
      Character.Id = CharacterId;
      Character.Data = CharacterDataObject;

      FRedwoodGetCharacterOutput Output;
      Output.Error = Error;
      Output.Character = Character;
      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodTitleGameSubsystem::SetSelectedCharacter(FString CharacterId) {
  SelectedCharacterId = CharacterId;
}

void URedwoodTitleGameSubsystem::JoinTicketing(
  TArray<FString> ModeIds,
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

  TicketingModeIds = ModeIds;
  TicketingRegions = InRegions;
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

  Server.ModeId = ServerProxy->GetStringField(TEXT("modeId"));

  Server.MapId = ServerProxy->GetStringField(TEXT("mapId"));

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
          URedwoodTitleGameSubsystem::ParseServerProxy(ServerData)
        );
      }

      FRedwoodListServersOutput Output;
      Output.Error = Error;
      Output.Servers = ServersStruct;
      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodTitleGameSubsystem::CreateServer(
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
  Payload->SetStringField(TEXT("mapId"), Parameters.MapId);

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
  } else {
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());
    Payload->SetField(TEXT("data"), NullValue);
  }

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

void URedwoodTitleGameSubsystem::GetServerInstance(
  FString ServerReference,
  FString Password,
  bool bJoinSession,
  FRedwoodGetServerOutputDelegate OnOutput
) {
  if (!Realm.IsValid() || !Realm->bIsConnected) {
    FRedwoodGetServerOutput Output;
    Output.Error = TEXT("Not connected to Realm.");
    OnOutput.ExecuteIfBound(Output);
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("id"), PlayerId);

  if (bJoinSession) {
    if (SelectedCharacterId.IsEmpty()) {
      FRedwoodGetServerOutput Output;
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

  Payload->SetStringField(TEXT("serverReference"), ServerReference);

  if (!Password.IsEmpty()) {
    Payload->SetStringField(TEXT("password"), Password);
  } else {
    TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull());
    Payload->SetField(TEXT("password"), NullValue);
  }

  Realm
    ->Emit(TEXT("realm:servers:get"), Payload, [this, OnOutput](auto Response) {
      FRedwoodGetServerOutput Output;

      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      Output.Error = MessageObject->GetStringField(TEXT("error"));

      const TSharedPtr<FJsonObject> *ServerInstanceObject = nullptr;
      if (MessageObject->TryGetObjectField(
            TEXT("instance"), ServerInstanceObject
          )) {
        FRedwoodGameServerInstance ServerInstance =
          URedwoodTitleGameSubsystem::ParseServerInstance(*ServerInstanceObject
          );
        Output.ServerInstance = ServerInstance;
      }

      OnOutput.ExecuteIfBound(Output);
    });
}

void URedwoodTitleGameSubsystem::StopServer(
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

  // TArray<FRedwoodRegionLatencySort> UnsortedRegions;
  // TArray<FRedwoodRegionLatencySort> SortedRegions;

  // for (auto Itr : Regions) {
  //   FRedwoodRegionLatencySort Region;
  //   Region.Id = Itr.Key;
  //   Region.RTT = PingAverages[Itr.Key];
  //   UnsortedRegions.Add(Region);
  // }

  // while (UnsortedRegions.Num() > 0) {
  //   int32 LowestIndex = 0;
  //   for (int32 i = 0; i < UnsortedRegions.Num(); i++) {
  //     if (UnsortedRegions[i].RTT < UnsortedRegions[LowestIndex].RTT) {
  //       LowestIndex = i;
  //     }
  //   }

  //   SortedRegions.Add(UnsortedRegions[LowestIndex]);
  //   UnsortedRegions.RemoveAt(LowestIndex);
  // }

  // TODO: Support multiple profiles
  // TArray<TSharedPtr<FJsonValue>> DesiredProfiles;
  // TSharedPtr<FJsonValueString> TicketingProfileValue =
  //   MakeShareable(new FJsonValueString(TicketingProfile));
  // DesiredProfiles.Add(TicketingProfileValue);

  // TArray<TSharedPtr<FJsonValue>> DesiredRegions;
  // for (FRedwoodRegionLatencySort Region : SortedRegions) {
  //   TSharedPtr<FJsonValueString> Value =
  //     MakeShareable(new FJsonValueString(Region.Id));
  //   DesiredRegions.Add(Value);
  // }

  JsonModern::FJson j = {
    {"playerId", PlayerId},
    {"characterId", SelectedCharacterId},
    {"data",
     {"modeIds", TicketingModeIds},
     {"regions", JsonModern::FJson::array()}}};

  for (FString DesiredRegion : TicketingRegions) {
    float *Ping = PingAverages.Find(DesiredRegion);
    if (Ping) {
      j["data"]["regions"].push_back({{"name", DesiredRegion}, {"ping", *Ping}}
      );
    }
  }

  // TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  // Payload->SetStringField(TEXT("playerId"), PlayerId);
  // Payload->SetStringField(TEXT("characterId"), SelectedCharacterId);

  // TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject);
  // Payload->SetArrayField(TEXT("profiles"), DesiredProfiles);
  // Payload->SetArrayField(TEXT("regions"), DesiredRegions);

  Realm->Emit(
    TEXT("realm:ticketing:join"),
    JsonModern::ToUnreal(j),
    [this](auto Response) {
      FString Error = Response[0]->AsString();
      FRedwoodTicketingUpdate Update;
      Update.Type = ERedwoodTicketingUpdateType::JoinResponse;
      Update.Message = Error;
      OnTicketingUpdate.ExecuteIfBound(Update);
    }
  );
}

FString URedwoodTitleGameSubsystem::GetConnectionConsoleCommand() {
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
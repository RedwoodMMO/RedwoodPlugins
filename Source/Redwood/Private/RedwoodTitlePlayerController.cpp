// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodTitlePlayerController.h"
#include "RedwoodModule.h"

#include "Kismet/KismetStringLibrary.h"

#include "LatencyCheckerLibrary.h"

ARedwoodTitlePlayerController::ARedwoodTitlePlayerController(
  const FObjectInitializer &ObjectInitializer
) :
  Super(ObjectInitializer) {
  DirectorSocketIOComponent = CreateDefaultSubobject<USocketIOClientComponent>(
    TEXT("DirectorSocketIOComponent")
  );
  DirectorSocketIOComponent->bShouldAutoConnect = false;
  PingAttemptsLeft = bUseWebsocketRegionPings ? 1 : PingAttempts;
}

void ARedwoodTitlePlayerController::BeginPlay() {
  Super::BeginPlay();

  if (DirectorSocketIOComponent) {
    DirectorSocketIOComponent->OnConnected.AddDynamic(
      this, &ARedwoodTitlePlayerController::HandleDirectorConnected
    );

    // TODO: add ability to load client.json config from disk?

    DirectorSocketIOComponent->Connect(DirectorAddress);
  }
}

void ARedwoodTitlePlayerController::HandleDirectorConnected(
  FString SocketId, FString SessionId, bool bIsReconnection
) {
  DirectorSocketIOComponent->OnNativeEvent(
    TEXT("realm:regions"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      HandleRegionsChanged(Event, Message);
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  DirectorSocketIOComponent->OnNativeEvent(
    TEXT("player:account-verified"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      TSharedPtr<FJsonObject> MessageObject = Message->AsObject();
      FString InPlayerId = MessageObject->GetStringField(TEXT("playerId"));

      if (InPlayerId == PlayerId) {
        OnAccountVerified.ExecuteIfBound(
          ERedwoodAuthUpdateType::Success, TEXT("")
        );
      }
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  DirectorSocketIOComponent->OnNativeEvent(
    TEXT("realm:lobby:update"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      FString UpdateMessage =
        Message->AsObject()->GetStringField(TEXT("message"));
      OnLobbyUpdate.ExecuteIfBound(
        ERedwoodLobbyUpdateType::Update, UpdateMessage
      );
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  DirectorSocketIOComponent->OnNativeEvent(
    TEXT("realm:lobby:ready"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      TSharedPtr<FJsonObject> MessageObject = Message->AsObject();

      LobbyConnection = MessageObject->GetStringField(TEXT("connection"));
      LobbyToken = MessageObject->GetStringField(TEXT("token"));

      OnLobbyUpdate.ExecuteIfBound(
        ERedwoodLobbyUpdateType::Ready, TEXT("The match is ready.")
      );
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  DirectorSocketIOComponent->OnNativeEvent(
    TEXT("realm:lobby:ticket-stale"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      OnLobbyUpdate.ExecuteIfBound(
        ERedwoodLobbyUpdateType::TicketStale,
        TEXT("Could not find a match. Please try again.")
      );
    },
    TEXT("/"),
    ESIOThreadOverrideOption::USE_GAME_THREAD
  );

  OnDirectorConnected.Broadcast(SocketId, SessionId, bIsReconnection);
}

void ARedwoodTitlePlayerController::HandleRegionsChanged(
  const FString &Event, const TSharedPtr<FJsonValue> &Message
) {
  FRegionsChanged MessageStruct;
  USIOJConvert::JsonObjectToUStruct(
    Message->AsObject(), FRegionsChanged::StaticStruct(), &MessageStruct, 0, 0
  );

  DataCenters.Empty();

  for (FRegion Region : MessageStruct.Regions) {
    TSharedPtr<FDataCenterLatency> DataCenter =
      MakeShareable(new FDataCenterLatency);
    DataCenter->Id = Region.Name;
    DataCenter->Url = Region.Ping;
    DataCenters.Add(DataCenter->Id, DataCenter);
  }

  GetWorld()->GetTimerManager().ClearTimer(PingTimer);
  InitiatePings();
}

void ARedwoodTitlePlayerController::InitiatePings() {
  FPingResult Delegate;
  Delegate.BindUFunction(this, FName(TEXT("HandlePingResult")));

  if (PingAttemptsLeft > 0) {
    for (auto Itr : DataCenters) {
      if (bUseWebsocketRegionPings || PingAttemptsLeft == PingAttempts) {
        // clear the last values
        Itr.Value->RTTs.Empty(PingAttempts);
      }

      if (bUseWebsocketRegionPings) {
        if (!Itr.Value->Url.StartsWith("ws")) {
          UE_LOG(
            LogRedwood,
            Error,
            TEXT("Expected websocket URL, got %s"),
            *Itr.Value->Url
          );
          continue;
        }
        ULatencyCheckerLibrary::PingWebSockets(
          Itr.Value->Url, PingTimeoutSec, PingAttempts, Delegate
        );
      } else {
        if (Itr.Value->Url.StartsWith("ws")) {
          UE_LOG(
            LogRedwood,
            Error,
            TEXT("Expected non-websocket URL, got %s"),
            *Itr.Value->Url
          );
          continue;
        }
        ULatencyCheckerLibrary::PingIcmp(
          Itr.Value->Url, PingTimeoutSec, Delegate
        );
      }
      PingAttemptsLeft--;
    }
  } else {
    // we're done pinging, let's store the averages
    for (auto Itr : DataCenters) {
      float Sum = 0;
      for (float RTT : Itr.Value->RTTs) {
        Sum += RTT;
      }
      float Average = Sum / Itr.Value->RTTs.Num();
      PingAverages.Add(Itr.Key, Average);

      OnPingResultReceived.Broadcast(
        Itr.Key, FMath::FloorToInt(Average * 1000)
      );
    }

    // queue the next set of pings
    PingAttemptsLeft = bUseWebsocketRegionPings ? 1 : PingAttempts;
    GetWorld()->GetTimerManager().SetTimer(
      PingTimer,
      this,
      &ARedwoodTitlePlayerController::InitiatePings,
      PingFrequencySec,
      false
    );
  }
}

void ARedwoodTitlePlayerController::HandlePingResult(
  FString TargetAddress, float RTT
) {
  for (auto Itr : DataCenters) {
    if (Itr.Value->Url == TargetAddress) {
      Itr.Value->RTTs.Add(RTT);
      break;
    }
  }

  for (auto Itr : DataCenters) {
    if (Itr.Value->Url.StartsWith("ws")) {
      // the LatencyChecker module handles averaging for us for websockets
      // and sends PingAttempts and provides a single number
      if (Itr.Value->RTTs.Num() == 0) {
        // we haven't finished receiving all of the pings for this round
        return;
      }
    } else {
      if (Itr.Value->RTTs.Num() + PingAttemptsLeft != PingAttempts) {
        // we haven't finished receiving all of the pings for this round
        return;
      }
    }
  }

  // we've received pings from all datacenters, ping again
  InitiatePings();
}

void ARedwoodTitlePlayerController::Register(
  const FString &Username, const FString &Password, FRedwoodAuthUpdate OnUpdated
) {
  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("username"), Username);
  Payload->SetStringField(TEXT("password"), Password);

  DirectorSocketIOComponent->EmitNative(
    TEXT("player:register:username"),
    Payload,
    [this, OnUpdated](auto Response) {
      TSharedPtr<FJsonObject> MessageStruct = Response[0]->AsObject();
      PlayerId = MessageStruct->GetStringField(TEXT("playerId"));
      FString Error = MessageStruct->GetStringField(TEXT("error"));

      if (Error.IsEmpty()) {
        OnUpdated.ExecuteIfBound(ERedwoodAuthUpdateType::Success, TEXT(""));
      } else if (Error == "Must verify account") {
        OnAccountVerified = OnUpdated;
        OnUpdated.ExecuteIfBound(
          ERedwoodAuthUpdateType::MustVerifyAccount, TEXT("")
        );
      } else {
        OnUpdated.ExecuteIfBound(ERedwoodAuthUpdateType::Error, Error);
      }
    }
  );
}

void ARedwoodTitlePlayerController::Login(
  const FString &Username,
  const FString &PasswordOrToken,
  FRedwoodAuthUpdate OnUpdated
) {
  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("username"), Username);
  Payload->SetStringField(TEXT("secret"), PasswordOrToken);

  DirectorSocketIOComponent->EmitNative(
    TEXT("player:login:username"),
    Payload,
    [this, OnUpdated](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      PlayerId = MessageObject->GetStringField(TEXT("playerId"));
      AuthToken = MessageObject->GetStringField(TEXT("token"));

      if (Error.IsEmpty()) {
        OnUpdated.ExecuteIfBound(ERedwoodAuthUpdateType::Success, TEXT(""));
      } else if (Error == "Must verify account") {
        OnAccountVerified = OnUpdated;
        OnUpdated.ExecuteIfBound(
          ERedwoodAuthUpdateType::MustVerifyAccount, TEXT("")
        );
      } else {
        OnUpdated.ExecuteIfBound(ERedwoodAuthUpdateType::Error, Error);
      }
    }
  );
}

void ARedwoodTitlePlayerController::CancelWaitingForAccountVerification() {
  if (OnAccountVerified.IsBound()) {
    OnAccountVerified.Clear();
  }
}

void ARedwoodTitlePlayerController::ListCharacters(
  FRedwoodCharactersResponse OnResponse
) {
  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);

  DirectorSocketIOComponent->EmitNative(
    TEXT("realm:characters:list"),
    Payload,
    [this, OnResponse](auto Response) {
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

      OnResponse.ExecuteIfBound(Error, CharactersStruct);
    }
  );
}

void ARedwoodTitlePlayerController::CreateCharacter(
  USIOJsonObject *Data, FRedwoodCharacterResponse OnResponse
) {
  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetObjectField(TEXT("data"), Data->GetRootObject());

  DirectorSocketIOComponent->EmitNative(
    TEXT("realm:characters:create"),
    Payload,
    [this, OnResponse](auto Response) {
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

      OnResponse.ExecuteIfBound(Error, Character);
    }
  );
}

void ARedwoodTitlePlayerController::GetCharacterData(
  FString CharacterId, FRedwoodCharacterResponse OnResponse
) {
  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("characterId"), CharacterId);

  DirectorSocketIOComponent->EmitNative(
    TEXT("realm:characters:get"),
    Payload,
    [this, OnResponse, CharacterId](auto Response) {
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

      OnResponse.ExecuteIfBound(Error, Character);
    }
  );
}

void ARedwoodTitlePlayerController::SetCharacterData(
  FString CharacterId,
  USIOJsonObject *Data,
  FRedwoodCharacterResponse OnResponse
) {
  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("characterId"), CharacterId);
  Payload->SetObjectField(TEXT("data"), Data->GetRootObject());

  DirectorSocketIOComponent->EmitNative(
    TEXT("realm:characters:set"),
    Payload,
    [this, OnResponse, CharacterId](auto Response) {
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

      OnResponse.ExecuteIfBound(Error, Character);
    }
  );
}

void ARedwoodTitlePlayerController::JoinLobby(
  FString Profile, FRedwoodLobbyUpdate OnUpdate
) {
  LobbyProfile = Profile;
  OnLobbyUpdate = OnUpdate;
  AttemptJoinLobby();
}

void ARedwoodTitlePlayerController::AttemptJoinLobby() {
  if (PingAverages.Num() != DataCenters.Num()) {
    // we haven't finished getting a single set of ping averages yet
    // let's delay sending our join request
    UE_LOG(
      LogRedwood,
      Log,
      TEXT(
        "Not enough ping averages. Num averages: %d, num datacenters: %d. Will attempt again."
      ),
      PingAverages.Num(),
      DataCenters.Num()
    );
    GetWorld()->GetTimerManager().SetTimer(
      PingTimer,
      this,
      &ARedwoodTitlePlayerController::AttemptJoinLobby,
      2.f,
      false
    );
    return;
  }

  TArray<FDataCenterLatencySort> UnsortedDataCenters;
  TArray<FDataCenterLatencySort> SortedDataCenters;

  for (auto Itr : DataCenters) {
    FDataCenterLatencySort DataCenter;
    DataCenter.Id = Itr.Key;
    DataCenter.RTT = PingAverages[Itr.Key];
    UnsortedDataCenters.Add(DataCenter);
  }

  while (UnsortedDataCenters.Num() > 0) {
    int32 LowestIndex = 0;
    for (int32 i = 0; i < UnsortedDataCenters.Num(); i++) {
      if (UnsortedDataCenters[i].RTT < UnsortedDataCenters[LowestIndex].RTT) {
        LowestIndex = i;
      }
    }

    SortedDataCenters.Add(UnsortedDataCenters[LowestIndex]);
    UnsortedDataCenters.RemoveAt(LowestIndex);
  }

  // TODO: Support multiple profiles
  TArray<TSharedPtr<FJsonValue>> DesiredProfiles;
  TSharedPtr<FJsonValueString> LobbyProfileValue =
    MakeShareable(new FJsonValueString(LobbyProfile));
  DesiredProfiles.Add(LobbyProfileValue);

  TArray<TSharedPtr<FJsonValue>> DesiredRegions;
  for (FDataCenterLatencySort DataCenter : SortedDataCenters) {
    TSharedPtr<FJsonValueString> Value =
      MakeShareable(new FJsonValueString(DataCenter.Id));
    DesiredRegions.Add(Value);
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetArrayField(TEXT("profiles"), DesiredProfiles);
  Payload->SetArrayField(TEXT("regions"), DesiredRegions);

  DirectorSocketIOComponent
    ->EmitNative(TEXT("realm:lobby:join"), Payload, [this](auto Response) {
      FString Error = Response[0]->AsString();
      OnLobbyUpdate.ExecuteIfBound(
        ERedwoodLobbyUpdateType::JoinResponse, Error
      );
    });
}

FString ARedwoodTitlePlayerController::GetMatchConnectionString(
  FString CharacterId
) {
  if (LobbyConnection.IsEmpty() || LobbyToken.IsEmpty()) {
    UE_LOG(
      LogRedwood,
      Error,
      TEXT(
        "Lobby connection or token is empty. Did you call JoinLobby and got an update that was of type Ready?"
      )
    );
    return "";
  }

  TMap<FString, FString> Options;
  Options.Add("RedwoodAuth", "1");
  Options.Add("PlayerId", PlayerId);
  Options.Add("CharacterId", CharacterId);
  Options.Add("Token", LobbyToken);

  TArray<FString> JoinedOptions;
  for (const TPair<FString, FString> &Option : Options) {
    JoinedOptions.Add(Option.Key + "=" + Option.Value);
  }

  FString OptionsString =
    UKismetStringLibrary::JoinStringArray(JoinedOptions, "?");
  FString ConnectionString = LobbyConnection + "?" + OptionsString;

  return ConnectionString;
}
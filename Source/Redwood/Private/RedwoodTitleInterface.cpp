// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodTitleInterface.h"
#include "RedwoodGameplayTags.h"
#include "RedwoodSaveGame.h"
#include "RedwoodSettings.h"
#include "RedwoodTitleGameSubsystem.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/KismetStringLibrary.h"
#include "LatencyCheckerLibrary.h"
#include "Misc/DateTime.h"
#include "SocketIOClient.h"
#include "TimerManager.h"

#include "JsonModern.h"

void URedwoodTitleInterface::Deinitialize() {
  if (Director.IsValid()) {
    ISocketIOClientModule::Get().ReleaseNativePointer(Director);
    Director = nullptr;
  }

  if (Realm.IsValid()) {
    ISocketIOClientModule::Get().ReleaseNativePointer(Realm);
    Realm = nullptr;
  }
}

void URedwoodTitleInterface::Tick(float DeltaTime) {
  TimerManager.Tick(DeltaTime);
}

TStatId URedwoodTitleInterface::GetStatId() const {
  RETURN_QUICK_DECLARE_CYCLE_STAT(URedwoodTitleInterface, STATGROUP_Tickables);
}

void URedwoodTitleInterface::InitializeDirectorConnection(
  FRedwoodSocketConnectedDelegate OnDirectorConnected
) {
  Director = ISocketIOClientModule::Get().NewValidNativePointer();

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

  Director->OnConnectedCallback = [OnDirectorConnected](
                                    const FString &InSocketId,
                                    const FString &InSessionId
                                  ) {
    FRedwoodSocketConnected Details;
    Details.Error = TEXT("");
    OnDirectorConnected.ExecuteIfBound(Details);
  };

  URedwoodSettings *RedwoodSettings = GetMutableDefault<URedwoodSettings>();
  Director->Connect(*RedwoodSettings->DirectorUri);
}

bool URedwoodTitleInterface::IsDirectorConnected() {
  return Director.IsValid() && Director->bIsConnected;
}

void URedwoodTitleInterface::HandleRegionsChanged(
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

void URedwoodTitleInterface::InitiatePings() {
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

    TimerManager.SetTimer(
      PingTimer,
      this,
      &URedwoodTitleInterface::InitiatePings,
      RedwoodSettings->PingFrequency,
      false
    );
  }
}

void URedwoodTitleInterface::HandlePingResult(
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

void URedwoodTitleInterface::Register(
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

void URedwoodTitleInterface::Logout() {
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

bool URedwoodTitleInterface::IsLoggedIn() {
  return !PlayerId.IsEmpty() && !AuthToken.IsEmpty();
}

FString URedwoodTitleInterface::GetPlayerId() {
  return PlayerId;
}

void URedwoodTitleInterface::AttemptAutoLogin(
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
    Login(SaveGame->Username, SaveGame->AuthToken, true, OnUpdate);
  } else {
    FRedwoodAuthUpdate Update;
    Update.Type = ERedwoodAuthUpdateType::Error;
    Update.Message = TEXT("No saved credentials found.");
    OnUpdate.ExecuteIfBound(Update);
  }
}

void URedwoodTitleInterface::Login(
  const FString &Username,
  const FString &PasswordOrToken,
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

  // JsonModern::FJson Payload = {
  //   {"username", Username}, {"secret", PasswordOrToken}};

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("username"), Username);
  Payload->SetStringField(TEXT("secret"), PasswordOrToken);

  Director->Emit(
    TEXT("player:login:username"),
    Payload,
    [this, Username, bRememberMe, OnUpdate](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));
      PlayerId = MessageObject->GetStringField(TEXT("playerId"));
      AuthToken = MessageObject->GetStringField(TEXT("token"));

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

void URedwoodTitleInterface::CancelWaitingForAccountVerification() {
  if (OnAccountVerified.IsBound()) {
    OnAccountVerified.Unbind();
  }
}

void URedwoodTitleInterface::ListRealms(
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
      OutRealm.Secret = RealmObj->GetStringField(TEXT("secret"));

      Output.Realms.Add(OutRealm);
    }

    OnOutput.ExecuteIfBound(Output);
  });
}

void URedwoodTitleInterface::InitializeConnectionForFirstRealm(
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

void URedwoodTitleInterface::InitializeRealmConnection(
  FRedwoodRealm InRealm, FRedwoodSocketConnectedDelegate OnRealmConnected
) {
  InitiateRealmHandshake(InRealm, OnRealmConnected);
}

void URedwoodTitleInterface::InitiateRealmHandshake(
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

      Realm->OnConnectedCallback =
        [this, Token, OnRealmConnected](
          const FString &InSocketId, const FString &InSessionId
        ) { FinalizeRealmHandshake(Token, OnRealmConnected); };

      Realm->Connect(*InRealm.Uri);
    }
  );
}

void URedwoodTitleInterface::FinalizeRealmHandshake(
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

void URedwoodTitleInterface::BindRealmEvents() {
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

bool URedwoodTitleInterface::IsRealmConnected() {
  return Realm.IsValid() && Realm->bIsConnected;
}

TMap<FString, float> URedwoodTitleInterface::GetRegions() {
  return PingAverages;
}

FRedwoodCharacter URedwoodTitleInterface::ParseCharacter(
  TSharedPtr<FJsonObject> CharacterObj
) {
  FRedwoodCharacter Character;
  Character.Id = CharacterObj->GetStringField(TEXT("id"));

  FDateTime::ParseIso8601(
    *CharacterObj->GetStringField(TEXT("createdAt")), Character.CreatedAt
  );

  FDateTime::ParseIso8601(
    *CharacterObj->GetStringField(TEXT("updatedAt")), Character.UpdatedAt
  );

  Character.PlayerId = CharacterObj->GetStringField(TEXT("playerId"));

  const TSharedPtr<FJsonObject> *CharacterMetadata = nullptr;
  if (CharacterObj->TryGetObjectField(TEXT("metadata"), CharacterMetadata)) {
    Character.Metadata = NewObject<USIOJsonObject>();
    Character.Metadata->SetRootObject(*CharacterMetadata);
  }

  const TSharedPtr<FJsonObject> *CharacterEquippedInventory = nullptr;
  if (CharacterObj->TryGetObjectField(
        TEXT("equippedInventory"), CharacterEquippedInventory
      )) {
    Character.EquippedInventory = NewObject<USIOJsonObject>();
    Character.EquippedInventory->SetRootObject(*CharacterEquippedInventory);
  }

  const TSharedPtr<FJsonObject> *NonequippedInventory = nullptr;
  if (CharacterObj->TryGetObjectField(
        TEXT("nonequippedInventory"), NonequippedInventory
      )) {
    Character.NonequippedInventory = NewObject<USIOJsonObject>();
    Character.NonequippedInventory->SetRootObject(*NonequippedInventory);
  }

  const TSharedPtr<FJsonObject> *CharacterData = nullptr;
  if (CharacterObj->TryGetObjectField(TEXT("data"), CharacterData)) {
    Character.Data = NewObject<USIOJsonObject>();
    Character.Data->SetRootObject(*CharacterData);
  }

  return Character;
}

void URedwoodTitleInterface::ListCharacters(
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

        CharactersStruct.Add(ParseCharacter(CharacterData));
      }

      FRedwoodListCharactersOutput Output;
      Output.Error = Error;
      Output.Characters = CharactersStruct;
      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodTitleInterface::CreateCharacter(
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
    FRedwoodGetCharacterOutput Output;
    Output.Error = TEXT(
      "Metadata must be a valid non-null object; it can be an empty object but it must exist. Other USIOJsonObject parameters can be null."
    );
    OnOutput.ExecuteIfBound(Output);
    return;
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
    TEXT("realm:characters:create"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      FRedwoodGetCharacterOutput Output;
      Output.Error = Error;

      const TSharedPtr<FJsonObject> *CharacterObj;
      if (MessageObject->TryGetObjectField(TEXT("character"), CharacterObj)) {
        Output.Character = ParseCharacter(*CharacterObj);
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodTitleInterface::GetCharacterData(
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
        Output.Character = ParseCharacter(*CharacterObj);
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodTitleInterface::SetCharacterData(
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

      FRedwoodGetCharacterOutput Output;
      Output.Error = Error;

      const TSharedPtr<FJsonObject> *CharacterObj;
      if (MessageObject->TryGetObjectField(TEXT("character"), CharacterObj)) {
        Output.Character = ParseCharacter(*CharacterObj);
      }

      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodTitleInterface::SetSelectedCharacter(FString CharacterId) {
  SelectedCharacterId = CharacterId;
}

void URedwoodTitleInterface::JoinTicketing(
  TArray<FString> ProfileTypes,
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

  TicketingProfileTypes = ProfileTypes;
  TicketingRegions = InRegions;
  OnTicketingUpdate = OnUpdate;
  AttemptJoinTicketing();
}

void URedwoodTitleInterface::CancelTicketing(
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
    TEXT("realm:ticketing:cancel"),
    Payload,
    [this, OnOutput](auto Response) {
      TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
      FString Error = MessageObject->GetStringField(TEXT("error"));

      OnOutput.ExecuteIfBound(Error);
    }
  );
}

FRedwoodGameServerProxy URedwoodTitleInterface::ParseServerProxy(
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

  FString EndedAt;
  if (ServerProxy->TryGetStringField(TEXT("endedAt"), EndedAt)) {
    FDateTime::ParseIso8601(*EndedAt, Server.EndedAt);
  }

  Server.Name = ServerProxy->GetStringField(TEXT("name"));

  Server.Region = ServerProxy->GetStringField(TEXT("region"));

  Server.ModeId = ServerProxy->GetStringField(TEXT("modeId"));

  Server.MapId = ServerProxy->GetStringField(TEXT("mapId"));

  Server.bPublic = ServerProxy->GetBoolField(TEXT("public"));

  Server.bProxyEndsWhenInstanceEnds =
    ServerProxy->GetBoolField(TEXT("proxyEndsWhenInstanceEnds"));

  Server.bContinuousPlay = ServerProxy->GetBoolField(TEXT("continuousPlay"));

  ServerProxy->TryGetBoolField(TEXT("hasPassword"), Server.bHasPassword);

  ServerProxy->TryGetStringField(TEXT("password"), Server.Password);

  ServerProxy->TryGetStringField(TEXT("shortCode"), Server.ShortCode);

  Server.CurrentPlayers = ServerProxy->GetIntegerField(TEXT("currentPlayers"));

  Server.MaxPlayers = ServerProxy->GetIntegerField(TEXT("maxPlayers"));

  const TSharedPtr<FJsonObject> *Data;
  if (ServerProxy->TryGetObjectField(TEXT("data"), Data)) {
    USIOJsonObject *DataObject = NewObject<USIOJsonObject>();
    DataObject->SetRootObject(*Data);
    Server.Data = DataObject;
  }

  ServerProxy->TryGetStringField(TEXT("ownerPlayerId"), Server.OwnerPlayerId);

  ServerProxy->TryGetStringField(
    TEXT("activeInstanceId"), Server.ActiveInstanceId
  );

  return Server;
}

FRedwoodGameServerInstance URedwoodTitleInterface::ParseServerInstance(
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

  FString StartedAt;
  if (ServerInstance->TryGetStringField(TEXT("startedAt"), StartedAt)) {
    FDateTime::ParseIso8601(*StartedAt, Instance.StartedAt);
  }

  FString EndedAt;
  if (ServerInstance->TryGetStringField(TEXT("endedAt"), EndedAt)) {
    FDateTime::ParseIso8601(*EndedAt, Instance.EndedAt);
  }

  ServerInstance->TryGetStringField(TEXT("connection"), Instance.Connection);

  Instance.ContainerId = ServerInstance->GetStringField(TEXT("containerId"));

  Instance.ProxyId = ServerInstance->GetStringField(TEXT("proxyId"));

  return Instance;
}

void URedwoodTitleInterface::ListServers(
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
        ServersStruct.Add(URedwoodTitleInterface::ParseServerProxy(ServerData));
      }

      FRedwoodListServersOutput Output;
      Output.Error = Error;
      Output.Servers = ServersStruct;
      OnOutput.ExecuteIfBound(Output);
    }
  );
}

void URedwoodTitleInterface::CreateServer(
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

  Payload->SetBoolField(
    TEXT("proxyEndsWhenInstanceEnds"), Parameters.bProxyEndsWhenInstanceEnds
  );

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

void URedwoodTitleInterface::GetServerInstance(
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
        FRedwoodGameServerInstance Instance =
          URedwoodTitleInterface::ParseServerInstance(*ServerInstanceObject);
        Output.Instance = Instance;
      }

      MessageObject->TryGetStringField(TEXT("token"), Output.Token);

      OnOutput.ExecuteIfBound(Output);
    });
}

void URedwoodTitleInterface::StopServer(
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

void URedwoodTitleInterface::AttemptJoinTicketing() {
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
      PingTimer, this, &URedwoodTitleInterface::AttemptJoinTicketing, 2.f, false
    );
    return;
  }

  TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
  Payload->SetStringField(TEXT("playerId"), PlayerId);
  Payload->SetStringField(TEXT("characterId"), SelectedCharacterId);

  TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject);

  TArray<TSharedPtr<FJsonValue>> DesiredProfileTypes;
  for (FString ProfileType : TicketingProfileTypes) {
    TSharedPtr<FJsonValueString> Value =
      MakeShareable(new FJsonValueString(ProfileType));
    DesiredProfileTypes.Add(Value);
  }
  Data->SetArrayField(TEXT("profileTypes"), DesiredProfileTypes);

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
  Data->SetArrayField(TEXT("regions"), DesiredRegions);

  Payload->SetObjectField(TEXT("data"), Data);

  Realm->Emit(TEXT("realm:ticketing:join"), Payload, [this](auto Response) {
    TSharedPtr<FJsonObject> MessageObject = Response[0]->AsObject();
    FString Error = MessageObject->GetStringField(TEXT("error"));

    FRedwoodTicketingUpdate Update;
    Update.Type = ERedwoodTicketingUpdateType::JoinResponse;
    Update.Message = Error;
    OnTicketingUpdate.ExecuteIfBound(Update);
  });
}

FString URedwoodTitleInterface::GetConnectionConsoleCommand() {
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
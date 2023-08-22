// Copyright Incanta Games 2023. All rights reserved.

#include "RedwoodGameMode.h"
#include "RedwoodSettings.h"
#include "RedwoodGameplayTags.h"

#include "Misc/Guid.h"
#include "Kismet/GameplayStatics.h"
#include "Dom/JsonObject.h"
#include "Net/OnlineEngineInterface.h"

#include "SocketIOClient.h"

void ARedwoodGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) {
  Super::InitGame(MapName, Options, ErrorMessage);

  URedwoodSettings* RedwoodSettings = GetMutableDefault<URedwoodSettings>();

  if (!GIsEditor || RedwoodSettings->bConnectToSidecarInPIE) {
    Sidecar = ISocketIOClientModule::Get().NewValidNativePointer();

    Sidecar->OnEvent(
      TEXT("realm:game-server:player-auth:response"),
      [this](const FString& Event, const TSharedPtr<FJsonValue>& Message)
      {
        const TSharedPtr<FJsonObject>* ObjectPtr;
        if (Message->TryGetObject(ObjectPtr))
        {
          const TSharedPtr<FJsonObject> Object = *ObjectPtr;
          FString Error = Object->GetStringField(TEXT("error"));
          FString RequestId = Object->GetStringField(TEXT("messageId"));
          if (PendingAuthenticationRequests.Contains(RequestId))
          {
            APlayerController* PlayerController = PendingAuthenticationRequests[RequestId];
            if (Error.IsEmpty())
            {
                TSharedPtr<FJsonObject> Character = Object->GetObjectField(TEXT("character"));
                FString CharacterId = Character->GetStringField(TEXT("id"));
                FString DisplayName = Character->GetStringField(TEXT("displayName"));
                TSharedPtr<FJsonObject> CharacterData = Character->GetObjectField(TEXT("data"));
                USIOJsonObject* CharacterJsonData = USIOJsonObject::ConstructJsonObject(this);
                CharacterJsonData->SetRootObject(CharacterData);

                FUniqueNetIdWrapper UniqueNetIdWrapper =
                  UOnlineEngineInterface::Get()->CreateUniquePlayerIdWrapper(
                    CharacterId,
                    FName(TEXT("RedwoodMMO"))
                  );
                FUniqueNetIdRepl UniqueId(UniqueNetIdWrapper.GetUniqueNetId());

                PlayerController->PlayerState->SetUniqueId(UniqueId);
                PlayerController->PlayerState->SetPlayerName(DisplayName);

                UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
                MessageSubsystem.BroadcastMessage(TAG_Redwood_Player_Joined, FRedwoodPlayerJoined{PlayerController->PlayerState, CharacterJsonData});
            }
            else
            {
              // kick the player
              PlayerController->ClientWasKicked(FText::FromString(Error));
            }

            PendingAuthenticationRequests.Remove(RequestId);
          }
        }
      }
    );

    // Sidecar will always be on the same host; 3020 is the default port
    Sidecar->Connect(TEXT("ws://127.0.0.1:3020"));
  }
}

APlayerController* ARedwoodGameMode::Login(
  UPlayer* NewPlayer,
  ENetRole InRemoteRole,
  const FString& Portal,
  const FString& Options,
  const FUniqueNetIdRepl& UniqueId,
  FString& ErrorMessage
) {
  APlayerController* PlayerController = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);

  URedwoodSettings* RedwoodSettings = GetMutableDefault<URedwoodSettings>();

  if (!GIsEditor || RedwoodSettings->bConnectToSidecarInPIE) {
    if (UGameplayStatics::HasOption(Options, TEXT("RedwoodAuth"))) {
      FString RequestId = FGuid::NewGuid().ToString();
      FString PlayerId = UGameplayStatics::ParseOption(Options, TEXT("PlayerId"));
      FString CharacterId = UGameplayStatics::ParseOption(Options, TEXT("CharacterId"));
      FString Token = UGameplayStatics::ParseOption(Options, TEXT("Token"));

      // set a map of request to player controller so we can set vars/kick
      PendingAuthenticationRequests.Add(RequestId, PlayerController);

      // query for player legitimacy
      TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
      JsonObject->SetStringField(TEXT("messageId"), RequestId);
      JsonObject->SetStringField(TEXT("playerId"), PlayerId);
      JsonObject->SetStringField(TEXT("characterId"), CharacterId);
      JsonObject->SetStringField(TEXT("token"), Token);
      Sidecar->Emit(TEXT("realm:game-server:player-auth"), JsonObject); // todo
    } else {
      ErrorMessage = TEXT("Invalid authentication request: missing RedwoodAuth option");
    }
  } else {
    UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
    USIOJsonObject* CharacterJsonData = USIOJsonObject::ConstructJsonObject(this);
    MessageSubsystem.BroadcastMessage(TAG_Redwood_Player_Joined, FRedwoodPlayerJoined{PlayerController->PlayerState, CharacterJsonData});
  }

  return PlayerController;
}

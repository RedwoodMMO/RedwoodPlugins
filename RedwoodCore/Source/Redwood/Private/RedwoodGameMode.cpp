// Copyright Incanta Games. All rights reserved.

#include "RedwoodGameMode.h"
#include "RedwoodGameSubsystem.h"
#include "RedwoodGameplayTags.h"

#if WITH_EDITOR
  #include "RedwoodEditorSettings.h"
#endif

#include "Dom/JsonObject.h"
#include "GameFramework/GameSession.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Guid.h"
#include "Net/OnlineEngineInterface.h"

#include "SocketIOClient.h"

void ARedwoodGameMode::InitGame(
  const FString &MapName, const FString &Options, FString &ErrorMessage
) {
  Super::InitGame(MapName, Options, ErrorMessage);

  bool bConnectToSidecar = false;

#if WITH_EDITOR
  URedwoodEditorSettings *RedwoodEditorSettings =
    GetMutableDefault<URedwoodEditorSettings>();
  bConnectToSidecar =
    !GIsEditor || RedwoodEditorSettings->bConnectToSidecarInPIE;
#else
  bConnectToSidecar = true;
#endif

  if (bConnectToSidecar) {
    Sidecar = ISocketIOClientModule::Get().NewValidNativePointer();

    URedwoodGameSubsystem *RedwoodGameSubsystem =
      GetGameInstance()->GetSubsystem<URedwoodGameSubsystem>();
    Sidecar->Connect(RedwoodGameSubsystem->SidecarUri);
  }
}

APlayerController *ARedwoodGameMode::Login(
  UPlayer *NewPlayer,
  ENetRole InRemoteRole,
  const FString &Portal,
  const FString &Options,
  const FUniqueNetIdRepl &UniqueId,
  FString &ErrorMessage
) {
  APlayerController *PlayerController = Super::Login(
    NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage
  );

  if (!ErrorMessage.IsEmpty() || PlayerController == nullptr) {
    return PlayerController;
  }

  bool bConnectToSidecar = false;

#if WITH_EDITOR
  URedwoodEditorSettings *RedwoodEditorSettings =
    GetMutableDefault<URedwoodEditorSettings>();
  bConnectToSidecar =
    !GIsEditor || RedwoodEditorSettings->bConnectToSidecarInPIE;
#else
  bConnectToSidecar = true;
#endif

  if (bConnectToSidecar) {
    if (UGameplayStatics::HasOption(Options, TEXT("RedwoodAuth"))) {
      FString PlayerId =
        UGameplayStatics::ParseOption(Options, TEXT("PlayerId"));
      FString CharacterId =
        UGameplayStatics::ParseOption(Options, TEXT("CharacterId"));
      FString Token = UGameplayStatics::ParseOption(Options, TEXT("Token"));

      // query for player legitimacy
      TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
      JsonObject->SetStringField(TEXT("playerId"), PlayerId);
      JsonObject->SetStringField(TEXT("characterId"), CharacterId);
      JsonObject->SetStringField(TEXT("token"), Token);

      Sidecar->Emit(
        TEXT("realm:servers:player-auth:game-server-to-sidecar"),
        JsonObject,
        [this, PlayerController](auto Response) {
          TSharedPtr<FJsonObject> MessageStruct = Response[0]->AsObject();
          FString Error = MessageStruct->GetStringField(TEXT("error"));

          if (Error.IsEmpty()) {
            TSharedPtr<FJsonObject> Character =
              MessageStruct->GetObjectField(TEXT("character"));
            FString CharacterId = Character->GetStringField(TEXT("id"));
            TSharedPtr<FJsonObject> CharacterData =
              Character->GetObjectField(TEXT("data"));
            USIOJsonObject *CharacterJsonData =
              USIOJsonObject::ConstructJsonObject(this);
            CharacterJsonData->SetRootObject(CharacterData);

            FUniqueNetIdWrapper UniqueNetIdWrapper =
              UOnlineEngineInterface::Get()->CreateUniquePlayerIdWrapper(
                CharacterId, FName(TEXT("RedwoodMMO"))
              );
            FUniqueNetIdRepl UniqueId(UniqueNetIdWrapper.GetUniqueNetId());

            PlayerController->PlayerState->SetUniqueId(UniqueId);

            FString Name;
            if (CharacterData->TryGetStringField(TEXT("name"), Name)) {
              PlayerController->PlayerState->SetPlayerName(Name);
            }

            UGameplayMessageSubsystem &MessageSubsystem =
              UGameplayMessageSubsystem::Get(this);
            MessageSubsystem.BroadcastMessage(
              TAG_Redwood_Player_Joined,
              FRedwoodPlayerJoined{
                PlayerController->PlayerState, CharacterJsonData}
            );
          } else {
            // kick the player
            UE_LOG(
              LogRedwood,
              Error,
              TEXT("Player failed to authenticate, kicking them now: %s"),
              *Error
            );
            GameSession->KickPlayer(PlayerController, FText::FromString(Error));
          }
        }
      );
    } else {
      ErrorMessage =
        TEXT("Invalid authentication request: missing RedwoodAuth option");
    }
  } else {
    UGameplayMessageSubsystem &MessageSubsystem =
      UGameplayMessageSubsystem::Get(this);
    USIOJsonObject *CharacterJsonData =
      USIOJsonObject::ConstructJsonObject(this);
    MessageSubsystem.BroadcastMessage(
      TAG_Redwood_Player_Joined,
      FRedwoodPlayerJoined{PlayerController->PlayerState, CharacterJsonData}
    );
  }

  return PlayerController;
}

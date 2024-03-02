// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodGameSubsystem.h"
#include "RedwoodGameplayTags.h"

#if WITH_EDITOR
  #include "RedwoodEditorSettings.h"
#endif

#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "SocketIOClient.h"

void URedwoodGameSubsystem::Initialize(FSubsystemCollectionBase &Collection) {
  Super::Initialize(Collection);

  UWorld *World = GetWorld();

  if (
    IsValid(World) &&
    (
      World->GetNetMode() == ENetMode::NM_DedicatedServer ||
      World->GetNetMode() == ENetMode::NM_ListenServer
    )
  ) {
#if WITH_EDITOR
    URedwoodEditorSettings *RedwoodEditorSettings =
      GetMutableDefault<URedwoodEditorSettings>();
    if (!GIsEditor || RedwoodEditorSettings->bConnectToSidecarInPIE) {
      InitializeSidecar();
    }
#else
    InitializeSidecar();
#endif

    UGameplayMessageSubsystem &MessageSubsystem =
      UGameplayMessageSubsystem::Get(this);
    ListenerHandle = MessageSubsystem.RegisterListener(
      TAG_Redwood_Shutdown_Instance,
      this,
      &URedwoodGameSubsystem::OnShutdownMessage
    );
  }
}

void URedwoodGameSubsystem::OnShutdownMessage(
  FGameplayTag Channel, const FRedwoodReason &Message
) {
  UE_LOG(
    LogRedwood,
    Log,
    TEXT("Received shutdown message, reason: %s"),
    *Message.Reason
  );
  bIsShuttingDown = true;
}

void URedwoodGameSubsystem::Deinitialize() {
  Super::Deinitialize();

  if (TimerHandle_UpdateSidecar.IsValid()) {
    GetGameInstance()->GetTimerManager().ClearTimer(TimerHandle_UpdateSidecar);
  }

  if (TimerHandle_UpdateSidecarLoading.IsValid()) {
    GetGameInstance()->GetTimerManager().ClearTimer(
      TimerHandle_UpdateSidecarLoading
    );
  }

  if (Sidecar.IsValid()) {
    ISocketIOClientModule::Get().ReleaseNativePointer(Sidecar);
    Sidecar = nullptr;
  }
}

void URedwoodGameSubsystem::InitializeSidecar() {
  Sidecar = ISocketIOClientModule::Get().NewValidNativePointer();

  Sidecar->OnEvent(
    TEXT("realm:game-server:load-map"),
    [this](const FString &Event, const TSharedPtr<FJsonValue> &Message) {
      UE_LOG(LogRedwood, Log, TEXT("Received message to load a map"));
      const TSharedPtr<FJsonObject> *Object;

      if (Message->TryGetObject(Object) && Object) {
        UE_LOG(LogRedwood, Log, TEXT("LoadMap message is valid object"));
        TSharedPtr<FJsonObject> ActualObject = *Object;
        TSharedPtr<FJsonValue> Map =
          ActualObject->GetField<EJson::String>(TEXT("map"));
        TSharedPtr<FJsonValue> Mode =
          ActualObject->GetField<EJson::String>(TEXT("mode"));

        if (Map.IsValid() && Mode.IsValid()) {
          FString MapStr = Map->AsString();
          FString ModeStr = Mode->AsString();
          UE_LOG(
            LogRedwood,
            Log,
            TEXT("LoadMap message has valid map (%s) and mode (%s)"),
            *MapStr,
            *ModeStr
          );
          FString Error;
          FURL Url;
          Url.Protocol = "unreal";
          Url.Map = MapStr;

          // TODO use Game= to specify a game mode
          Url.AddOption(*FString("Mode=" + ModeStr));

          FString Command = FString::Printf(TEXT("open %s"), *Url.ToString());
          GetGameInstance()->GetEngine()->DeferredCommands.Add(Command);
        }
      }
    }
  );

  Sidecar->OnConnectedCallback =
    [this](const FString &InSocketId, const FString &InSessionId) {
      if (Sidecar.IsValid()) {
        GetGameInstance()->GetTimerManager().SetTimer(
          TimerHandle_UpdateSidecarLoading,
          this,
          &URedwoodGameSubsystem::SendUpdateToSidecar,
          UpdateSidecarLoadingRate,
          true // loop
        );

        GetGameInstance()->GetTimerManager().SetTimer(
          TimerHandle_UpdateSidecar,
          this,
          &URedwoodGameSubsystem::SendUpdateToSidecar,
          UpdateSidecarRate,
          true, // loop
          0.f // immediately trigger first one
        );
      }
    };

  // Sidecar will always be on the same host; 3020 is the default port
  Sidecar->Connect(TEXT("ws://127.0.0.1:3020"));
}

void URedwoodGameSubsystem::SendUpdateToSidecar() {
  if (Sidecar.IsValid()) {
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> NumPlayersObject = MakeShareable(new FJsonObject);

    UWorld *World = GetWorld();

    if (IsValid(World)) {
      AGameModeBase *GameMode = World->GetAuthGameMode();
      if (IsValid(GameMode)) {
        bool bWorldStarted = World->GetRealTimeSeconds() > 0;
        if (bWorldStarted) {
          if (TimerHandle_UpdateSidecarLoading.IsValid()) {
            GetGameInstance()->GetTimerManager().ClearTimer(
              TimerHandle_UpdateSidecarLoading
            );
          }

          if (GameMode->HasMatchStarted()) {
            if (GameMode->HasMatchEnded()) {
              if (bIsShuttingDown) {
                JsonObject->SetStringField(TEXT("state"), TEXT("Stopping"));
              } else {
                JsonObject->SetStringField(TEXT("state"), TEXT("Ended"));
              }

              JsonObject->SetStringField(
                TEXT("playerAcceptance"), TEXT("NotAccepting")
              );
            } else {
              JsonObject->SetStringField(TEXT("state"), TEXT("Started"));
              JsonObject->SetStringField(
                TEXT("playerAcceptance"), TEXT("NotAccepting")
              ); // todo: support backfills
            }
          } else {
            JsonObject->SetStringField(
              TEXT("state"), TEXT("WaitingForPlayers")
            );
            JsonObject->SetStringField(
              TEXT("playerAcceptance"), TEXT("Public")
            );
          }
        } else {
          JsonObject->SetStringField(TEXT("state"), TEXT("LoadingMap"));
          JsonObject->SetStringField(
            TEXT("playerAcceptance"), TEXT("NotAccepting")
          );
        }

        JsonObject->SetStringField(TEXT("map"), World->URL.Map);

        JsonObject->SetStringField(
          TEXT("mode"), World->URL.GetOption(TEXT("Mode="), TEXT(""))
        );

        NumPlayersObject->SetNumberField(
          TEXT("current"), GameMode->GetNumPlayers()
        );
        NumPlayersObject->SetNumberField(
          TEXT("max"),
          GameMode->GameSession == nullptr ? 0
                                           : GameMode->GameSession->MaxPlayers
        );
        JsonObject->SetObjectField(TEXT("numPlayers"), NumPlayersObject);
      } else {
        bool bStarted = World->GetRealTimeSeconds() > 0;
        JsonObject->SetStringField(TEXT("state"), TEXT("LoadingMap"));
        JsonObject->SetStringField(
          TEXT("playerAcceptance"), TEXT("NotAccepting")
        );

        JsonObject->SetStringField(TEXT("map"), World->URL.Map);

        JsonObject->SetStringField(
          TEXT("mode"), World->URL.GetOption(TEXT("Mode="), TEXT(""))
        );

        NumPlayersObject->SetNumberField(TEXT("current"), 0);
        NumPlayersObject->SetNumberField(TEXT("max"), 0);
        JsonObject->SetObjectField(TEXT("numPlayers"), NumPlayersObject);
      }
    } else {
      JsonObject->SetStringField(TEXT("state"), TEXT("Starting"));
      JsonObject->SetStringField(TEXT("mode"), TEXT(""));
      JsonObject->SetStringField(TEXT("map"), TEXT(""));
      NumPlayersObject->SetNumberField(TEXT("current"), 0);
      NumPlayersObject->SetNumberField(TEXT("max"), 0);
      JsonObject->SetObjectField(TEXT("numPlayers"), NumPlayersObject);

      JsonObject->SetStringField(
        TEXT("playerAcceptance"), TEXT("NotAccepting")
      );
    }

    Sidecar->Emit(TEXT("realm:game-server:update"), JsonObject);
  }
}

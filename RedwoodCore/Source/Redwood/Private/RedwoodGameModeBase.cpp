// Copyright Incanta Games. All rights reserved.

#include "RedwoodGameModeBase.h"

#define REDWOOD_GAME_MODE_TYPE ARedwoodGameModeBase

REDWOOD_GAME_MODE_TYPE::
  REDWOOD_GAME_MODE_TYPE(const FObjectInitializer
                           &ObjectInitializer /*= FObjectInitializer::Get()*/) :
  Super(ObjectInitializer) {

  GameModeComponent =
    CreateDefaultSubobject<URedwoodGameModeComponent>(TEXT("GameModeComponent")
    );
}

void REDWOOD_GAME_MODE_TYPE::InitGame(
  const FString &MapName, const FString &Options, FString &ErrorMessage
) {
  Super::InitGame(MapName, Options, ErrorMessage);

  GameModeComponent->InitGame(MapName, Options, ErrorMessage);
}

APlayerController *REDWOOD_GAME_MODE_TYPE::Login(
  UPlayer *NewPlayer,
  ENetRole InRemoteRole,
  const FString &Portal,
  const FString &Options,
  const FUniqueNetIdRepl &UniqueId,
  FString &ErrorMessage
) {
  return GameModeComponent->Login(
    NewPlayer,
    InRemoteRole,
    Portal,
    Options,
    UniqueId,
    ErrorMessage,
    std::function<APlayerController
                    *(UPlayer *,
                      ENetRole,
                      const FString &,
                      const FString &,
                      const FUniqueNetIdRepl &,
                      FString &)>(
      [this](
        UPlayer *DelegateNewPlayer,
        ENetRole DelegateInRemoteRole,
        const FString &DelegatePortal,
        const FString &DelegateOptions,
        const FUniqueNetIdRepl &DelegateUniqueId,
        FString &DelegateErrorMessage
      ) -> APlayerController * {
        return Super::Login(
          DelegateNewPlayer,
          DelegateInRemoteRole,
          DelegatePortal,
          DelegateOptions,
          DelegateUniqueId,
          DelegateErrorMessage
        );
      }
    )
  );
}

bool REDWOOD_GAME_MODE_TYPE::PlayerCanRestart_Implementation(
  APlayerController *Player
) {
  return GameModeComponent->PlayerCanRestart_Implementation(
    Player,
    std::function<bool(APlayerController *)>(
      [this](APlayerController *DelegatePlayer) -> bool {
        return Super::PlayerCanRestart_Implementation(DelegatePlayer);
      }
    )
  );
}

void REDWOOD_GAME_MODE_TYPE::FinishRestartPlayer(
  AController *NewPlayer, const FRotator &StartRotation
) {
  GameModeComponent->FinishRestartPlayer(
    NewPlayer,
    StartRotation,
    std::function<void(AController *)>([this](AController *DelegateNewPlayer) {
      FailedToRestartPlayer(DelegateNewPlayer);
    })
  );
}

APawn *REDWOOD_GAME_MODE_TYPE::SpawnDefaultPawnAtTransform_Implementation(
  AController *NewPlayer, const FTransform &SpawnTransform
) {
  FTransform NewSpawnTransform =
    GameModeComponent->PickPawnSpawnTransform(NewPlayer, SpawnTransform);

  return Super::SpawnDefaultPawnAtTransform_Implementation(
    NewPlayer, NewSpawnTransform
  );
}

#undef REDWOOD_GAME_MODE_TYPE

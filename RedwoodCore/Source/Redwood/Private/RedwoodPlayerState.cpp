// Copyright Incanta LLC. All rights reserved.

#include "RedwoodPlayerState.h"
#include "RedwoodPlayerStateComponent.h"

ARedwoodPlayerState::
  ARedwoodPlayerState(const FObjectInitializer
                        &ObjectInitializer /*= FObjectInitializer::Get()*/) :
  Super(ObjectInitializer) {
#if WITH_EDITORONLY_DATA
  bIsSpatiallyLoaded = true;
#endif // WITH_EDITORONLY_DATA

  PrimaryActorTick.bCanEverTick = true;

  PlayerStateComponent = CreateDefaultSubobject<URedwoodPlayerStateComponent>(
    TEXT("PlayerStateComponent")
  );
  PlayerStateComponent->bFollowPawn = bFollowPawn;
  PlayerStateComponent->OnRedwoodCharacterUpdated.AddUniqueDynamic(
    this, &ARedwoodPlayerState::HandleCharacterUpdated
  );
  PlayerStateComponent->OnRedwoodPlayerUpdated.AddUniqueDynamic(
    this, &ARedwoodPlayerState::HandlePlayerUpdated
  );
}

void ARedwoodPlayerState::Tick(float DeltaSeconds) {
  Super::Tick(DeltaSeconds);
}

void ARedwoodPlayerState::SetClientReady_Implementation() {
  PlayerStateComponent->SetClientReady();
}

bool ARedwoodPlayerState::SetClientReady_Validate() {
  return true;
}

void ARedwoodPlayerState::SetServerReady() {
  PlayerStateComponent->SetServerReady();
}

void ARedwoodPlayerState::SetRedwoodPlayer(FRedwoodPlayerData InRedwoodPlayer) {
  PlayerStateComponent->SetRedwoodPlayer(InRedwoodPlayer);
}

void ARedwoodPlayerState::SetRedwoodCharacter(
  FRedwoodCharacterBackend InRedwoodCharacter
) {
  PlayerStateComponent->SetRedwoodCharacter(InRedwoodCharacter);
}

void ARedwoodPlayerState::HandleCharacterUpdated() {
  if (!IsValid(PlayerStateComponent)) {
    return;
  }
  RedwoodCharacter = PlayerStateComponent->RedwoodCharacter;
  OnRedwoodCharacterUpdated.Broadcast();
}

void ARedwoodPlayerState::HandlePlayerUpdated() {
  if (!IsValid(PlayerStateComponent)) {
    return;
  }
  RedwoodPlayer = PlayerStateComponent->RedwoodPlayer;
  OnRedwoodPlayerUpdated.Broadcast();
};
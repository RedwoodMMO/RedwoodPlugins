// Copyright Incanta LLC. All rights reserved.

#include "RedwoodPlayerState.h"

#include "Net/OnlineEngineInterface.h"

ARedwoodPlayerState::
  ARedwoodPlayerState(const FObjectInitializer
                        &ObjectInitializer /*= FObjectInitializer::Get()*/) :
  Super(ObjectInitializer) {
#if WITH_EDITORONLY_DATA
  bIsSpatiallyLoaded = true;
#endif // WITH_EDITORONLY_DATA

  PrimaryActorTick.bCanEverTick = true;
}

void ARedwoodPlayerState::Tick(float DeltaSeconds) {
  Super::Tick(DeltaSeconds);

  if (bFollowPawn) {
    UWorld *World = GetWorld();

    if (
      IsValid(World) &&
      (
        World->GetNetMode() == ENetMode::NM_DedicatedServer ||
        World->GetNetMode() == ENetMode::NM_ListenServer ||
        World->GetNetMode() == ENetMode::NM_Standalone
      )
    ) {
      APawn *Pawn = GetPawn();
      if (IsValid(Pawn)) {
        AActor::SetActorLocation(Pawn->GetActorLocation());
        AActor::SetActorRotation(Pawn->GetActorRotation());
      }
    }
  }
}

void ARedwoodPlayerState::SetClientReady_Implementation() {
  if (GetLocalRole() == ROLE_Authority) {
    bClientReady = true;
  }
}

bool ARedwoodPlayerState::SetClientReady_Validate() {
  return true;
}

void ARedwoodPlayerState::SetServerReady() {
  if (GetLocalRole() == ROLE_Authority) {
    bServerReady = true;
  }
}

void ARedwoodPlayerState::SetRedwoodCharacter(
  FRedwoodCharacterBackend InRedwoodCharacter
) {
  RedwoodCharacter = InRedwoodCharacter;

  FUniqueNetIdWrapper UniqueNetIdWrapper =
    UOnlineEngineInterface::Get()->CreateUniquePlayerIdWrapper(
      RedwoodCharacter.PlayerId + TEXT(":") + RedwoodCharacter.Id,
      FName(TEXT("RedwoodMMO"))
    );
  FUniqueNetIdRepl NetUniqueId(UniqueNetIdWrapper.GetUniqueNetId());

  SetUniqueId(NetUniqueId);

  SetPlayerName(RedwoodCharacter.Name);

  OnRedwoodCharacterUpdated.Broadcast();
}

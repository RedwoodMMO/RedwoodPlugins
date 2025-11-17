// Copyright Incanta LLC. All rights reserved.

#include "RedwoodPlayerStateComponent.h"
#include "RedwoodModule.h"
#include "RedwoodPlayerState.h"

#include "GameFramework/PlayerState.h"
#include "Net/OnlineEngineInterface.h"

URedwoodPlayerStateComponent::URedwoodPlayerStateComponent(
  const FObjectInitializer &ObjectInitializer /*= FObjectInitializer::Get()*/
) :
  Super(ObjectInitializer) {

  PrimaryComponentTick.bCanEverTick = true;

  OwnerPlayerState = Cast<APlayerState>(GetOwner());
  if (!OwnerPlayerState.IsValid()) {
    if (IsValid(GetOwner())) {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT(
          "RedwoodPlayerStateComponent requires a PlayerState owner, but attached to %s."
        ),
        *GetOwner()->GetName()
      );
    } else {
      UE_LOG(
        LogRedwood,
        Warning,
        TEXT(
          "RedwoodPlayerStateComponent requires a PlayerState owner, but this one has no owner (perhaps CDO)."
        )
      );
    }

    return;
  }

  if (Cast<ARedwoodPlayerState>(GetOwner())) {
    UE_LOG(
      LogRedwood,
      Warning,
      TEXT(
        "ARedwoodPlayerState is deprecated and will be removed in 5.0.0. Migrate to using URedwoodPlayerStateComponent on any other APlayerState actor."
      ),
      *GetOwner()->GetName()
    );
  }

  OwnerPlayerState->PrimaryActorTick.bCanEverTick = true;
}

void URedwoodPlayerStateComponent::TickComponent(
  float DeltaTime,
  enum ELevelTick TickType,
  FActorComponentTickFunction *ThisTickFunction
) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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
      if (APlayerState *PlayerState = OwnerPlayerState.Get()) {
        APawn *Pawn = PlayerState->GetPawn();
        AActor *PlayerStateActor = Cast<AActor>(PlayerState);
        if (IsValid(Pawn)) {
          PlayerStateActor->SetActorLocation(Pawn->GetActorLocation());
          PlayerStateActor->SetActorRotation(Pawn->GetActorRotation());
        }
      }
    }
  }
}

void URedwoodPlayerStateComponent::SetClientReady_Implementation() {
  if (AActor *Owner = GetOwner()) {
    if (Owner->GetLocalRole() == ROLE_Authority) {
      bClientReady = true;
    }
  }
}

bool URedwoodPlayerStateComponent::SetClientReady_Validate() {
  return true;
}

void URedwoodPlayerStateComponent::SetServerReady() {
  if (AActor *Owner = GetOwner()) {
    if (Owner->GetLocalRole() == ROLE_Authority) {
      bServerReady = true;
    }
  }
}

void URedwoodPlayerStateComponent::SetRedwoodPlayer(
  FRedwoodPlayerData InRedwoodPlayer
) {
  RedwoodPlayer = InRedwoodPlayer;

  OnRedwoodPlayerUpdated.Broadcast();
}

void URedwoodPlayerStateComponent::SetRedwoodCharacter(
  FRedwoodCharacterBackend InRedwoodCharacter
) {
  RedwoodCharacter = InRedwoodCharacter;

  if (APlayerState *PlayerState = OwnerPlayerState.Get()) {
    FUniqueNetIdWrapper UniqueNetIdWrapper =
      UOnlineEngineInterface::Get()->CreateUniquePlayerIdWrapper(
        RedwoodCharacter.PlayerId + TEXT(":") + RedwoodCharacter.Id,
        FName(TEXT("RedwoodMMO"))
      );
    FUniqueNetIdRepl NetUniqueId(UniqueNetIdWrapper.GetUniqueNetId());

    PlayerState->SetUniqueId(NetUniqueId);
  }

  OnRedwoodCharacterUpdated.Broadcast();
}

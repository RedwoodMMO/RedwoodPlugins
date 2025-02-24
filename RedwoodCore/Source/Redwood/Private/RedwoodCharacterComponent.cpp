// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodCharacterComponent.h"
#include "RedwoodCommonGameSubsystem.h"
#include "RedwoodModule.h"
#include "RedwoodPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "SIOJConvert.h"
#include "SIOJsonObject.h"

URedwoodCharacterComponent::URedwoodCharacterComponent(
  const FObjectInitializer &ObjectInitializer
) :
  Super(ObjectInitializer) {
  SetIsReplicatedByDefault(true);
}

void URedwoodCharacterComponent::GetLifetimeReplicatedProps(
  TArray<FLifetimeProperty> &OutLifetimeProps
) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(URedwoodCharacterComponent, RedwoodPlayerId);
  DOREPLIFETIME(URedwoodCharacterComponent, RedwoodCharacterId);
  DOREPLIFETIME(URedwoodCharacterComponent, RedwoodCharacterName);
}

void URedwoodCharacterComponent::BeginPlay() {
  Super::BeginPlay();

  APawn *Pawn = Cast<APawn>(GetOwner());

  if (Pawn) {
    Pawn->ReceiveControllerChangedDelegate.AddUniqueDynamic(
      this, &URedwoodCharacterComponent::OnControllerChanged
    );
    AController *Controller = Pawn->GetController();
    if (IsValid(Controller)) {
      OnControllerChanged(Pawn, nullptr, Controller);
    }
  }
}

void URedwoodCharacterComponent::OnControllerChanged(
  APawn *Pawn, AController *OldController, AController *NewController
) {
  if (IsValid(NewController)) {
    TObjectPtr<ARedwoodPlayerState> RedwoodPlayerState =
      Cast<ARedwoodPlayerState>(NewController->PlayerState);
    if (RedwoodPlayerState) {
      RedwoodPlayerState->OnRedwoodCharacterUpdated.AddUniqueDynamic(
        this, &URedwoodCharacterComponent::RedwoodPlayerStateCharacterUpdated
      );
      RedwoodPlayerStateCharacterUpdated();
    }
  }
}

void URedwoodCharacterComponent::RedwoodPlayerStateCharacterUpdated() {
  APawn *Pawn = Cast<APawn>(GetOwner());
  AController *Controller = IsValid(Pawn) ? Pawn->GetController() : nullptr;
  ARedwoodPlayerState *RedwoodPlayerState = IsValid(Controller)
    ? Cast<ARedwoodPlayerState>(Controller->PlayerState)
    : nullptr;
  if (IsValid(RedwoodPlayerState)) {
    FRedwoodCharacterBackend RedwoodCharacterBackend =
      RedwoodPlayerState->RedwoodCharacter;

    RedwoodPlayerId = RedwoodCharacterBackend.PlayerId;
    RedwoodCharacterId = RedwoodCharacterBackend.Id;

    if (bUseCharacterCreatorData) {
      bool bDirty = URedwoodCommonGameSubsystem::DeserializeBackendData(
        bStoreDataInActor ? (UObject *)Pawn : (UObject *)this,
        RedwoodCharacterBackend.CharacterCreatorData,
        *CharacterCreatorDataVariableName,
        LatestMetadataSchemaVersion
      );

      if (bDirty) {
        MarkCharacterCreatorDataDirty();
      }
    }

    if (bUseMetadata) {
      bool bDirty = URedwoodCommonGameSubsystem::DeserializeBackendData(
        bStoreDataInActor ? (UObject *)Pawn : (UObject *)this,
        RedwoodCharacterBackend.Metadata,
        *MetadataVariableName,
        LatestMetadataSchemaVersion
      );

      if (bDirty) {
        MarkMetadataDirty();
      }
    }

    if (bUseEquippedInventory) {
      bool bDirty = URedwoodCommonGameSubsystem::DeserializeBackendData(
        bStoreDataInActor ? (UObject *)Pawn : (UObject *)this,
        RedwoodCharacterBackend.EquippedInventory,
        *EquippedInventoryVariableName,
        LatestEquippedInventorySchemaVersion
      );

      if (bDirty) {
        MarkEquippedInventoryDirty();
      }
    }

    if (bUseNonequippedInventory) {
      bool bDirty = URedwoodCommonGameSubsystem::DeserializeBackendData(
        bStoreDataInActor ? (UObject *)Pawn : (UObject *)this,
        RedwoodCharacterBackend.NonequippedInventory,
        *NonequippedInventoryVariableName,
        LatestNonequippedInventorySchemaVersion
      );

      if (bDirty) {
        MarkNonequippedInventoryDirty();
      }
    }

    if (bUseData) {
      bool bDirty = URedwoodCommonGameSubsystem::DeserializeBackendData(
        bStoreDataInActor ? (UObject *)Pawn : (UObject *)this,
        RedwoodCharacterBackend.Data,
        *DataVariableName,
        LatestDataSchemaVersion
      );

      if (bDirty) {
        MarkDataDirty();
      }
    }

    OnRedwoodCharacterUpdated.Broadcast();
  }
}

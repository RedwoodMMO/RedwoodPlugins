// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodCharacter.h"
#include "RedwoodCommonGameSubsystem.h"
#include "RedwoodModule.h"
#include "RedwoodPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "SIOJConvert.h"
#include "SIOJsonObject.h"

ARedwoodCharacter::ARedwoodCharacter(const FObjectInitializer &ObjectInitializer
) :
  Super(ObjectInitializer) {
  SetReplicates(true);
  SetReplicateMovement(true);
}

void ARedwoodCharacter::GetLifetimeReplicatedProps(
  TArray<FLifetimeProperty> &OutLifetimeProps
) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(ARedwoodCharacter, RedwoodPlayerId);
  DOREPLIFETIME(ARedwoodCharacter, RedwoodCharacterId);
  DOREPLIFETIME(ARedwoodCharacter, RedwoodCharacterName);
}

void ARedwoodCharacter::BeginPlay() {
  Super::BeginPlay();
}

void ARedwoodCharacter::PossessedBy(AController *NewController) {
  Super::PossessedBy(NewController);

  ARedwoodPlayerState *RedwoodPlayerState =
    Cast<ARedwoodPlayerState>(GetPlayerState());
  if (RedwoodPlayerState) {
    RedwoodPlayerState->OnRedwoodCharacterUpdated.AddUniqueDynamic(
      this, &ARedwoodCharacter::RedwoodPlayerStateCharacterUpdated
    );
    RedwoodPlayerStateCharacterUpdated();
  }
}

void ARedwoodCharacter::RedwoodPlayerStateCharacterUpdated() {
  ARedwoodPlayerState *RedwoodPlayerState =
    Cast<ARedwoodPlayerState>(GetPlayerState());
  if (RedwoodPlayerState) {
    FRedwoodCharacterBackend RedwoodCharacterBackend =
      RedwoodPlayerState->RedwoodCharacter;

    RedwoodPlayerId = RedwoodCharacterBackend.PlayerId;
    RedwoodCharacterId = RedwoodCharacterBackend.Id;

    URedwoodCommonGameSubsystem::DeserializeBackendData(
      this,
      RedwoodCharacterBackend.CharacterCreatorData,
      *CharacterCreatorDataVariableName,
      LatestMetadataSchemaVersion
    );

    URedwoodCommonGameSubsystem::DeserializeBackendData(
      this,
      RedwoodCharacterBackend.Metadata,
      *MetadataVariableName,
      LatestMetadataSchemaVersion
    );

    URedwoodCommonGameSubsystem::DeserializeBackendData(
      this,
      RedwoodCharacterBackend.EquippedInventory,
      *EquippedInventoryVariableName,
      LatestEquippedInventorySchemaVersion
    );

    URedwoodCommonGameSubsystem::DeserializeBackendData(
      this,
      RedwoodCharacterBackend.NonequippedInventory,
      *NonequippedInventoryVariableName,
      LatestNonequippedInventorySchemaVersion
    );

    URedwoodCommonGameSubsystem::DeserializeBackendData(
      this,
      RedwoodCharacterBackend.Data,
      *DataVariableName,
      LatestDataSchemaVersion
    );

    OnRedwoodCharacterUpdated();
  }
}

void ARedwoodCharacter::OnRedwoodCharacterUpdated_Implementation() {
  //
}

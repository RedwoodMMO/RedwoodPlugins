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
  DOREPLIFETIME(URedwoodCharacterComponent, RedwoodPlayerNickname);
  DOREPLIFETIME(URedwoodCharacterComponent, RedwoodNameTag);
  DOREPLIFETIME_CONDITION(
    URedwoodCharacterComponent, bSelectedGuildValid, COND_OwnerOnly
  );
  DOREPLIFETIME_CONDITION(
    URedwoodCharacterComponent, SelectedGuild, COND_OwnerOnly
  );
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
  } else {
    ARedwoodPlayerState *RedwoodPlayerState =
      Cast<ARedwoodPlayerState>(GetOwner());

    if (IsValid(RedwoodPlayerState)) {
      RedwoodPlayerState->OnRedwoodPlayerUpdated.AddUniqueDynamic(
        this, &URedwoodCharacterComponent::RedwoodPlayerStatePlayerUpdated
      );
      RedwoodPlayerStatePlayerUpdated();

      RedwoodPlayerState->OnRedwoodCharacterUpdated.AddUniqueDynamic(
        this, &URedwoodCharacterComponent::RedwoodPlayerStateCharacterUpdated
      );
      RedwoodPlayerStateCharacterUpdated();
    } else {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT(
          "URedwoodCharacterComponent must be used with APawn or ARedwoodPlayerState"
        )
      );
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
      RedwoodPlayerState->OnRedwoodPlayerUpdated.AddUniqueDynamic(
        this, &URedwoodCharacterComponent::RedwoodPlayerStatePlayerUpdated
      );
      RedwoodPlayerStatePlayerUpdated();

      RedwoodPlayerState->OnRedwoodCharacterUpdated.AddUniqueDynamic(
        this, &URedwoodCharacterComponent::RedwoodPlayerStateCharacterUpdated
      );
      RedwoodPlayerStateCharacterUpdated();
    }
  }
}

void URedwoodCharacterComponent::RedwoodPlayerStatePlayerUpdated() {
  APawn *Pawn = Cast<APawn>(GetOwner());
  AController *Controller = IsValid(Pawn) ? Pawn->GetController() : nullptr;
  ARedwoodPlayerState *RedwoodPlayerState = IsValid(Controller)
    ? Cast<ARedwoodPlayerState>(Controller->PlayerState)
    : Cast<ARedwoodPlayerState>(GetOwner());
  if (IsValid(RedwoodPlayerState)) {
    FRedwoodPlayerData PlayerData = RedwoodPlayerState->RedwoodPlayer;

    RedwoodPlayerNickname = PlayerData.Nickname;
    RedwoodNameTag = PlayerData.bSelectedGuildValid
      ? PlayerData.SelectedGuild.Guild.Tag
      : FString();

    bSelectedGuildValid = PlayerData.bSelectedGuildValid;
    SelectedGuild = PlayerData.SelectedGuild;

    if (bUsePlayerData) {
      bool bDirty = URedwoodCommonGameSubsystem::DeserializeBackendData(
        bStoreDataInActor ? (UObject *)Pawn : (UObject *)this,
        PlayerData.Data,
        *PlayerDataVariableName,
        LatestMetadataSchemaVersion
      );

      if (bDirty) {
        MarkPlayerDataDirty();
      }
    }

    FString FormatPlayerNameFunctionName = TEXT("FormatPlayerName");
    UFunction *FormatPlayerNameFunction =
      GetOwner()->GetClass()->FindFunctionByName(*FormatPlayerNameFunctionName);
    FString *CustomPlayerName = nullptr;

    if (FormatPlayerNameFunction) {
      // Ensure the function is valid and has the correct signature
      if (
        !FormatPlayerNameFunction->IsValidLowLevel() ||
        FormatPlayerNameFunction->NumParms != 1 ||
        !FormatPlayerNameFunction->ReturnValueOffset
      ) {
        UE_LOG(
          LogRedwood,
          Error,
          TEXT(
            "Function %s in %s has an invalid signature, using default player name."
          ),
          *FormatPlayerNameFunctionName,
          *GetOwner()->GetName()
        );
      } else {
        // Allocate memory for the parameters
        void *Params = FMemory::Malloc(FormatPlayerNameFunction->ParmsSize);
        FMemory::Memzero(Params, FormatPlayerNameFunction->ParmsSize);

        FProperty *FunctionStructProp = FormatPlayerNameFunction->PropertyLink;
        FProperty *FunctionObjectProp = FunctionStructProp->PropertyLinkNext;

        // Call the function
        GetOwner()->ProcessEvent(FormatPlayerNameFunction, Params);

        // Retrieve the return value
        void *ReturnValue =
          (void
             *)((SIZE_T)Params + FormatPlayerNameFunction->ReturnValueOffset);

        // Copy the return value to CustomPlayerName
        CustomPlayerName = (FString *)ReturnValue;

        // Clean up
        FMemory::Free(Params);
      }
    }

    FString DefaultPlayerName = RedwoodNameTag.IsEmpty()
      ? PlayerData.Nickname
      : FString::Printf(TEXT("[%s] %s"), *RedwoodNameTag, *PlayerData.Nickname);

    RedwoodPlayerState->SetPlayerName(
      CustomPlayerName == nullptr ? DefaultPlayerName : *CustomPlayerName
    );

    OnRedwoodPlayerUpdated.Broadcast();
    MC_RedwoodPlayerUpdated();
  }
}

void URedwoodCharacterComponent::MC_RedwoodPlayerUpdated_Implementation() {
  OnRedwoodPlayerUpdated.Broadcast();
}

void URedwoodCharacterComponent::RedwoodPlayerStateCharacterUpdated() {
  APawn *Pawn = Cast<APawn>(GetOwner());
  AController *Controller = IsValid(Pawn) ? Pawn->GetController() : nullptr;
  ARedwoodPlayerState *RedwoodPlayerState = IsValid(Controller)
    ? Cast<ARedwoodPlayerState>(Controller->PlayerState)
    : Cast<ARedwoodPlayerState>(GetOwner());
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
    MC_RedwoodCharacterUpdated();
  }
}

void URedwoodCharacterComponent::MC_RedwoodCharacterUpdated_Implementation() {
  OnRedwoodCharacterUpdated.Broadcast();
}

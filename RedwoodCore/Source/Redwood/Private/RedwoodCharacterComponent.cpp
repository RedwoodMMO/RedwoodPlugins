// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodCharacterComponent.h"
#include "RedwoodCommonGameSubsystem.h"
#include "RedwoodModule.h"
#include "RedwoodPlayerStateComponent.h"

#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
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
    if (IsValid(Controller) && IsValid(Controller->PlayerState)) {
      OnControllerChanged(Pawn, nullptr, Controller);
    }
  } else {
    APlayerState *PlayerState = Cast<APlayerState>(GetOwner());

    if (IsValid(PlayerState)) {
      URedwoodPlayerStateComponent *PlayerStateComponent =
        PlayerState->FindComponentByClass<URedwoodPlayerStateComponent>();

      if (IsValid(PlayerStateComponent)) {
        PlayerStateComponent->OnRedwoodPlayerUpdated.AddUniqueDynamic(
          this, &URedwoodCharacterComponent::RedwoodPlayerStatePlayerUpdated
        );
        RedwoodPlayerStatePlayerUpdated();

        PlayerStateComponent->OnRedwoodCharacterUpdated.AddUniqueDynamic(
          this, &URedwoodCharacterComponent::RedwoodPlayerStateCharacterUpdated
        );
        RedwoodPlayerStateCharacterUpdated();
      } else {
        UE_LOG(
          LogRedwood,
          Error,
          TEXT(
            "URedwoodCharacterComponent requires the owning APlayerState to have an attached URedwoodPlayerStateComponent"
          )
        );
      }
    } else {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT(
          "URedwoodCharacterComponent must be used with APawn or APlayerState"
        )
      );
    }
  }
}

void URedwoodCharacterComponent::OnControllerChanged(
  APawn *Pawn, AController *OldController, AController *NewController
) {
  if (IsValid(NewController)) {
    URedwoodPlayerStateComponent *PlayerStateComponent =
      NewController->PlayerState
        ->FindComponentByClass<URedwoodPlayerStateComponent>();
    if (IsValid(PlayerStateComponent)) {
      PlayerStateComponent->OnRedwoodPlayerUpdated.AddUniqueDynamic(
        this, &URedwoodCharacterComponent::RedwoodPlayerStatePlayerUpdated
      );
      RedwoodPlayerStatePlayerUpdated();

      PlayerStateComponent->OnRedwoodCharacterUpdated.AddUniqueDynamic(
        this, &URedwoodCharacterComponent::RedwoodPlayerStateCharacterUpdated
      );
      RedwoodPlayerStateCharacterUpdated();
    }
  }
}

void URedwoodCharacterComponent::RedwoodPlayerStatePlayerUpdated() {
  APawn *Pawn = Cast<APawn>(GetOwner());
  AController *Controller = IsValid(Pawn) ? Pawn->GetController() : nullptr;
  APlayerState *PlayerState = IsValid(Controller)
    ? Cast<APlayerState>(Controller->PlayerState)
    : Cast<APlayerState>(GetOwner());

  URedwoodPlayerStateComponent *PlayerStateComponent = IsValid(PlayerState)
    ? PlayerState->FindComponentByClass<URedwoodPlayerStateComponent>()
    : nullptr;

  if (IsValid(PlayerStateComponent)) {
    FRedwoodPlayerData PlayerData = PlayerStateComponent->RedwoodPlayer;

    RedwoodPlayerNickname = PlayerData.Nickname;
    RedwoodNameTag = PlayerData.bSelectedGuildValid
      ? PlayerData.SelectedGuild.Guild.Tag
      : FString();

    bSelectedGuildValid = PlayerData.bSelectedGuildValid;
    SelectedGuild = PlayerData.SelectedGuild;

    if (bUsePlayerData) {
      bool bErrored = false;
      bool bDirty = URedwoodCommonGameSubsystem::DeserializeBackendData(
        bStoreDataInActor ? (UObject *)Pawn : (UObject *)this,
        PlayerData.Data,
        *PlayerDataVariableName,
        LatestMetadataSchemaVersion,
        bErrored
      );

      if (bErrored) {
        // kick the player as they're not compatible with the server
        APlayerController *PlayerController =
          PlayerState->GetPlayerController();

        if (IsValid(PlayerController)) {
          AGameModeBase *GameMode = UGameplayStatics::GetGameMode(PlayerState);
          if (IsValid(GameMode) && GameMode->GameSession) {
            GameMode->GameSession->KickPlayer(
              PlayerController,
              FText::FromString(TEXT("Could not load player character data"))
            );
          }
        }
      }

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

    PlayerState->SetPlayerName(
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
  APlayerState *PlayerState = IsValid(Controller)
    ? Cast<APlayerState>(Controller->PlayerState)
    : Cast<APlayerState>(GetOwner());

  URedwoodPlayerStateComponent *PlayerStateComponent = IsValid(PlayerState)
    ? PlayerState->FindComponentByClass<URedwoodPlayerStateComponent>()
    : nullptr;

  if (IsValid(PlayerStateComponent)) {
    FRedwoodCharacterBackend RedwoodCharacterBackend =
      PlayerStateComponent->RedwoodCharacter;

    RedwoodPlayerId = RedwoodCharacterBackend.PlayerId;
    RedwoodCharacterId = RedwoodCharacterBackend.Id;
    RedwoodCharacterName = RedwoodCharacterBackend.Name;

    if (bUseCharacterCreatorData) {
      bool bErrored = false;
      bool bDirty = URedwoodCommonGameSubsystem::DeserializeBackendData(
        bStoreDataInActor ? (UObject *)Pawn : (UObject *)this,
        RedwoodCharacterBackend.CharacterCreatorData,
        *CharacterCreatorDataVariableName,
        LatestMetadataSchemaVersion,
        bErrored
      );

      if (bErrored) {
        // kick the player as they're not compatible with the server
        APlayerController *PlayerController =
          PlayerState->GetPlayerController();

        if (IsValid(PlayerController)) {
          AGameModeBase *GameMode = UGameplayStatics::GetGameMode(PlayerState);
          if (IsValid(GameMode) && GameMode->GameSession) {
            GameMode->GameSession->KickPlayer(
              PlayerController,
              FText::FromString(TEXT("Could not load player character data"))
            );
          }
        }
      }

      if (bDirty) {
        MarkCharacterCreatorDataDirty();
      }
    }

    if (bUseMetadata) {
      bool bErrored = false;
      bool bDirty = URedwoodCommonGameSubsystem::DeserializeBackendData(
        bStoreDataInActor ? (UObject *)Pawn : (UObject *)this,
        RedwoodCharacterBackend.Metadata,
        *MetadataVariableName,
        LatestMetadataSchemaVersion,
        bErrored
      );

      if (bErrored) {
        // kick the player as they're not compatible with the server
        APlayerController *PlayerController =
          PlayerState->GetPlayerController();

        if (IsValid(PlayerController)) {
          AGameModeBase *GameMode = UGameplayStatics::GetGameMode(PlayerState);
          if (IsValid(GameMode) && GameMode->GameSession) {
            GameMode->GameSession->KickPlayer(
              PlayerController,
              FText::FromString(TEXT("Could not load player character data"))
            );
          }
        }
      }

      if (bDirty) {
        MarkMetadataDirty();
      }
    }

    if (bUseEquippedInventory) {
      bool bErrored = false;
      bool bDirty = URedwoodCommonGameSubsystem::DeserializeBackendData(
        bStoreDataInActor ? (UObject *)Pawn : (UObject *)this,
        RedwoodCharacterBackend.EquippedInventory,
        *EquippedInventoryVariableName,
        LatestEquippedInventorySchemaVersion,
        bErrored
      );

      if (bErrored) {
        // kick the player as they're not compatible with the server
        APlayerController *PlayerController =
          PlayerState->GetPlayerController();

        if (IsValid(PlayerController)) {
          AGameModeBase *GameMode = UGameplayStatics::GetGameMode(PlayerState);
          if (IsValid(GameMode) && GameMode->GameSession) {
            GameMode->GameSession->KickPlayer(
              PlayerController,
              FText::FromString(TEXT("Could not load player character data"))
            );
          }
        }
      }

      if (bDirty) {
        MarkEquippedInventoryDirty();
      }
    }

    if (bUseNonequippedInventory) {
      bool bErrored = false;
      bool bDirty = URedwoodCommonGameSubsystem::DeserializeBackendData(
        bStoreDataInActor ? (UObject *)Pawn : (UObject *)this,
        RedwoodCharacterBackend.NonequippedInventory,
        *NonequippedInventoryVariableName,
        LatestNonequippedInventorySchemaVersion,
        bErrored
      );

      if (bErrored) {
        // kick the player as they're not compatible with the server
        APlayerController *PlayerController =
          PlayerState->GetPlayerController();

        if (IsValid(PlayerController)) {
          AGameModeBase *GameMode = UGameplayStatics::GetGameMode(PlayerState);
          if (IsValid(GameMode) && GameMode->GameSession) {
            GameMode->GameSession->KickPlayer(
              PlayerController,
              FText::FromString(TEXT("Could not load player character data"))
            );
          }
        }
      }

      if (bDirty) {
        MarkNonequippedInventoryDirty();
      }
    }

    if (bUseProgress) {
      bool bErrored = false;
      bool bDirty = URedwoodCommonGameSubsystem::DeserializeBackendData(
        bStoreDataInActor ? (UObject *)Pawn : (UObject *)this,
        RedwoodCharacterBackend.Progress,
        *ProgressVariableName,
        LatestProgressSchemaVersion,
        bErrored
      );

      if (bErrored) {
        // kick the player as they're not compatible with the server
        APlayerController *PlayerController =
          PlayerState->GetPlayerController();

        if (IsValid(PlayerController)) {
          AGameModeBase *GameMode = UGameplayStatics::GetGameMode(PlayerState);
          if (IsValid(GameMode) && GameMode->GameSession) {
            GameMode->GameSession->KickPlayer(
              PlayerController,
              FText::FromString(TEXT("Could not load player character data"))
            );
          }
        }
      }

      if (bDirty) {
        MarkProgressDirty();
      }
    }

    if (bUseData) {
      bool bErrored = false;
      bool bDirty = URedwoodCommonGameSubsystem::DeserializeBackendData(
        bStoreDataInActor ? (UObject *)Pawn : (UObject *)this,
        RedwoodCharacterBackend.Data,
        *DataVariableName,
        LatestDataSchemaVersion,
        bErrored
      );

      if (bErrored) {
        // kick the player as they're not compatible with the server
        APlayerController *PlayerController =
          PlayerState->GetPlayerController();

        if (IsValid(PlayerController)) {
          AGameModeBase *GameMode = UGameplayStatics::GetGameMode(PlayerState);
          if (IsValid(GameMode) && GameMode->GameSession) {
            GameMode->GameSession->KickPlayer(
              PlayerController,
              FText::FromString(TEXT("Could not load player character data"))
            );
          }
        }
      }

      if (bDirty) {
        MarkDataDirty();
      }
    }

    if (bUseAbilitySystem) {
      bool bErrored = false;
      bool bDirty = URedwoodCommonGameSubsystem::DeserializeBackendData(
        bStoreDataInActor ? (UObject *)Pawn : (UObject *)this,
        RedwoodCharacterBackend.AbilitySystem,
        *AbilitySystemVariableName,
        LatestAbilitySystemSchemaVersion,
        bErrored
      );

      if (bErrored) {
        // kick the player as they're not compatible with the server
        APlayerController *PlayerController =
          PlayerState->GetPlayerController();

        if (IsValid(PlayerController)) {
          AGameModeBase *GameMode = UGameplayStatics::GetGameMode(PlayerState);
          if (IsValid(GameMode) && GameMode->GameSession) {
            GameMode->GameSession->KickPlayer(
              PlayerController,
              FText::FromString(TEXT("Could not load player character data"))
            );
          }
        }
      }

      if (bDirty) {
        MarkAbilitySystemDirty();
      }
    }

    OnRedwoodCharacterUpdated.Broadcast();
    MC_RedwoodCharacterUpdated();
  }
}

void URedwoodCharacterComponent::MC_RedwoodCharacterUpdated_Implementation() {
  OnRedwoodCharacterUpdated.Broadcast();
}

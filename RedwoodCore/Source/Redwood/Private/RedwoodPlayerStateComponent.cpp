// Copyright Incanta LLC. All rights reserved.

#include "RedwoodPlayerStateComponent.h"
#include "RedwoodModule.h"
#include "RedwoodPlayerState.h"
#include "RedwoodServerGameSubsystem.h"
#include "RedwoodZoneSpawn.h"

#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/OnlineEngineInterface.h"

URedwoodPlayerStateComponent::URedwoodPlayerStateComponent(
  const FObjectInitializer &ObjectInitializer
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

bool URedwoodPlayerStateComponent::GetSpawnData(
  FTransform &Transform, FRotator &ControlRotation
) {
  if (RedwoodCharacter.RedwoodData != nullptr && RedwoodCharacter.RedwoodData->IsValid()) {
    URedwoodServerGameSubsystem *RedwoodServerGameSubsystem =
      GetWorld()->GetGameInstance()->GetSubsystem<URedwoodServerGameSubsystem>(
      );

    if (!RedwoodServerGameSubsystem) {
      return false;
    }

    USIOJsonObject *LastLocation;
    if (RedwoodCharacter.RedwoodData->TryGetObjectField(
          TEXT("lastLocation"), LastLocation
        )) {
      FString LastZoneName = LastLocation->GetStringField(TEXT("zoneName"));

      if (LastZoneName != RedwoodServerGameSubsystem->ZoneName) {
        return false;
      }

      FString LastSpawnName;
      if (LastLocation->TryGetStringField(TEXT("spawnName"), LastSpawnName)) {
        TArray<AActor *> ZoneSpawns;
        UGameplayStatics::GetAllActorsOfClass(
          GetWorld(), ARedwoodZoneSpawn::StaticClass(), ZoneSpawns
        );

        TArray<ARedwoodZoneSpawn *> RedwoodZoneSpawns;
        for (AActor *ZoneSpawn : ZoneSpawns) {
          ARedwoodZoneSpawn *RedwoodZoneSpawn =
            Cast<ARedwoodZoneSpawn>(ZoneSpawn);
          if (IsValid(RedwoodZoneSpawn)) {
            if (RedwoodZoneSpawn->ZoneName == RedwoodServerGameSubsystem->ZoneName && RedwoodZoneSpawn->SpawnName == LastSpawnName) {
              FTransform SpawnTransform = RedwoodZoneSpawn->GetSpawnTransform();
              Transform.SetLocation(SpawnTransform.GetLocation());
              Transform.SetRotation(SpawnTransform.GetRotation());
              Transform.SetScale3D(SpawnTransform.GetScale3D());

              FRotator SpawnRotation = SpawnTransform.GetRotation().Rotator();
              ControlRotation.SetComponentForAxis(
                EAxis::X, SpawnRotation.GetComponentForAxis(EAxis::X)
              );
              ControlRotation.SetComponentForAxis(
                EAxis::Y, SpawnRotation.GetComponentForAxis(EAxis::Y)
              );
              ControlRotation.SetComponentForAxis(
                EAxis::Z, SpawnRotation.GetComponentForAxis(EAxis::Z)
              );
              return true;
            }
          }
        }

        return false;
      }

      USIOJsonObject *LastTransform;

      if (LastLocation->TryGetObjectField(TEXT("transform"), LastTransform)) {
        USIOJsonObject *Location;
        if (!LastTransform->TryGetObjectField(TEXT("location"), Location)) {
          UE_LOG(
            LogRedwood,
            Error,
            TEXT("Invalid lastTransform (no location object field)")
          );

          return false;
        }

        USIOJsonObject *Rotation;
        if (!LastTransform->TryGetObjectField(TEXT("rotation"), Rotation)) {
          UE_LOG(
            LogRedwood,
            Error,
            TEXT("Invalid lastTransform (no rotation object field)")
          );

          return false;
        }

        float LocX = Location->GetNumberField(TEXT("x"));
        float LocY = Location->GetNumberField(TEXT("y"));
        float LocZ = Location->GetNumberField(TEXT("z"));

        float RotX = Rotation->GetNumberField(TEXT("x"));
        float RotY = Rotation->GetNumberField(TEXT("y"));
        float RotZ = Rotation->GetNumberField(TEXT("z"));

        Transform =
          FTransform(FRotator(RotY, RotZ, RotX), FVector(LocX, LocY, LocZ));

        USIOJsonObject *ControlRotationObject;
        if (!LastTransform->TryGetObjectField(
              TEXT("controlRotation"), ControlRotationObject
            )) {
          UE_LOG(
            LogRedwood,
            Error,
            TEXT("Invalid lastTransform (no controlRotation object field)")
          );

          return false;
        }

        float Roll = ControlRotationObject->GetNumberField(TEXT("x"));
        float Pitch = ControlRotationObject->GetNumberField(TEXT("y"));
        float Yaw = ControlRotationObject->GetNumberField(TEXT("z"));

        ControlRotation = FRotator(Pitch, Yaw, Roll);

        return true;
      }
    }
  }

  return false;
}

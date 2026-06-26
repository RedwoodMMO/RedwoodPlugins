// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodGameStateComponent.h"
#include "RedwoodServerGameSubsystem.h"

#include "Net/Core/PushModel/PushModel.h"
#include "Net/UnrealNetwork.h"

URedwoodGameStateComponent::URedwoodGameStateComponent(
  const FObjectInitializer &ObjectInitializer
) :
  Super(ObjectInitializer) {
  SetIsReplicatedByDefault(true);
}

void URedwoodGameStateComponent::GetLifetimeReplicatedProps(
  TArray<FLifetimeProperty> &OutLifetimeProps
) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  FDoRepLifetimeParams Params;
  Params.bIsPushBased = true;
  DOREPLIFETIME_WITH_PARAMS_FAST(
    URedwoodGameStateComponent, ServerDetails, Params
  );
}

void URedwoodGameStateComponent::BeginPlay() {
  Super::BeginPlay();

  UWorld *World = GetWorld();

  if (IsValid(World) && (
      World->GetNetMode() == ENetMode::NM_DedicatedServer ||
      World->GetNetMode() == ENetMode::NM_ListenServer
    )) {
    URedwoodServerGameSubsystem *RedwoodServerGameSubsystem =
      World->GetGameInstance()->GetSubsystem<URedwoodServerGameSubsystem>();

    if (IsValid(RedwoodServerGameSubsystem)) {
      SetServerDetails(
        RedwoodServerGameSubsystem->RealmName,
        RedwoodServerGameSubsystem->ProxyId,
        RedwoodServerGameSubsystem->InstanceId,
        RedwoodServerGameSubsystem->ZoneName,
        RedwoodServerGameSubsystem->ShardName
      );
    } else {
      UE_LOG(
        LogRedwood, Warning, TEXT("RedwoodServerGameSubsystem is invalid.")
      );
      SetServerDetails(TEXT(""), TEXT(""), TEXT(""), TEXT(""), TEXT(""));
    }
  }
}

void URedwoodGameStateComponent::SetServerDetails(
  FString RealmName,
  FString ProxyId,
  FString InstanceId,
  FString ZoneName,
  FString ShardName
) {
  FRedwoodServerDetails Details;
  Details.RealmName = RealmName;
  Details.ProxyId = ProxyId;
  Details.InstanceId = InstanceId;
  Details.ZoneName = ZoneName;
  Details.ShardName = ShardName;
  ServerDetails = Details;
  MARK_PROPERTY_DIRTY_FROM_NAME(
    URedwoodGameStateComponent, ServerDetails, this
  );
}

void URedwoodGameStateComponent::OnRep_ServerDetails() {
  OnServerDetailsChanged.Broadcast();
}

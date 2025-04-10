// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodGameStateComponent.h"
#include "RedwoodServerGameSubsystem.h"

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

  DOREPLIFETIME(URedwoodGameStateComponent, ServerDetails);
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
        RedwoodServerGameSubsystem->ZoneName,
        RedwoodServerGameSubsystem->ShardName
      );
    } else {
      UE_LOG(
        LogRedwood, Warning, TEXT("RedwoodServerGameSubsystem is invalid.")
      );
      SetServerDetails(TEXT(""), TEXT(""), TEXT(""), TEXT(""));
    }
  }
}

void URedwoodGameStateComponent::SetServerDetails(
  FString RealmName, FString ProxyId, FString ZoneName, FString ShardName
) {
  FRedwoodServerDetails Details;
  Details.RealmName = RealmName;
  Details.ProxyId = ProxyId;
  Details.ZoneName = ZoneName;
  Details.ShardName = ShardName;
  ServerDetails = Details;
}

void URedwoodGameStateComponent::OnRep_ServerDetails() {
  OnServerDetailsChanged.Broadcast();
}

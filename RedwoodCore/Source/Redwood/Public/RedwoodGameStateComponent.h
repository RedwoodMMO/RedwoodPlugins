// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Types/RedwoodTypes.h"

#include "RedwoodGameStateComponent.generated.h"

UCLASS(
  Blueprintable,
  BlueprintType,
  ClassGroup = (Redwood),
  meta = (BlueprintSpawnableComponent)
)
class REDWOOD_API URedwoodGameStateComponent : public UActorComponent {
  GENERATED_BODY()

public:
  URedwoodGameStateComponent(const FObjectInitializer &ObjectInitializer);

  //~UActorComponent interface
  virtual void BeginPlay() override;
  //~End of UActorComponent interface

  void SetServerDetails(
    FString RealmName, FString ProxyId, FString ZoneName, FString ShardName
  );

  UFUNCTION(BlueprintPure, Category = "Redwood")
  FRedwoodServerDetails GetServerDetails() const {
    return ServerDetails;
  }

  UPROPERTY(BlueprintAssignable, Category = "Redwood")
  FRedwoodDynamicDelegate OnServerDetailsChanged;

private:
  UFUNCTION()
  void OnRep_ServerDetails();

  UPROPERTY(ReplicatedUsing = OnRep_ServerDetails)
  FRedwoodServerDetails ServerDetails;
};

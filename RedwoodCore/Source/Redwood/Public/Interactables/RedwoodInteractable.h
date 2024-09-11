// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RedwoodGameplayTags.h"

#include "GameFramework/GameplayMessageSubsystem.h"

#include "RedwoodInteractable.generated.h"

class USphereComponent;
class ARedwoodCharacter;

UCLASS(BlueprintType, Blueprintable)
class REDWOOD_API ARedwoodInteractable : public AActor {
  GENERATED_BODY()

public:
  ARedwoodInteractable();

  //~ Begin AActor Interface
  void BeginPlay() override;
  //~ End AActor Interface

  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Redwood")
  void OnInteract(ARedwoodCharacter *Character);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Redwood")
  USphereComponent *SphereComponent;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Redwood")
  bool bAutoInteract = false;

protected:
  virtual void GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps
  ) const override;

private:
  FGameplayMessageListenerHandle ListenerHandle;
  void OnInteraction(
    FGameplayTag InChannel, const FRedwoodPlayerInteraction &Message
  );
};

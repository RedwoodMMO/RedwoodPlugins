// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "RedwoodAbilitySystemComponent.generated.h"

UCLASS(
  Blueprintable,
  BlueprintType,
  ClassGroup = (Redwood),
  meta = (BlueprintSpawnableComponent)
)
class REDWOODGAS_API URedwoodAbilitySystemComponent
  : public UAbilitySystemComponent {
  GENERATED_BODY()

public:
  URedwoodAbilitySystemComponent(const FObjectInitializer &ObjectInitializer);

  //~ Begin UActorComponent Interface
  virtual void BeginPlay() override;
  //~ End UActorComponent Interface

  UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Redwood")
  void MarkDirty() {
    bDirty = true;
  }

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  bool IsDirty() const {
    return bDirty;
  }

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void DoSerialize();

  UFUNCTION(BlueprintCallable, Category = "Redwood")
  void DoDeserialize();

protected:
  TSharedPtr<FJsonObject> SerializeASC();
  void DeserializeASC(TSharedPtr<FJsonObject> Data);

private:
  UFUNCTION()
  void OnControllerChanged(
    APawn *Pawn, AController *OldController, AController *NewController
  );

  UFUNCTION()
  void RedwoodPlayerStateCharacterUpdated();

  bool bDirty = false;
};

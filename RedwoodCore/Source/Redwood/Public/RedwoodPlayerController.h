// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "RedwoodPlayerController.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Redwood))
class REDWOOD_API ARedwoodPlayerController : public APlayerController {
  GENERATED_BODY()

public:
  //~APlayerController interface
  virtual void ClientSetRotation_Implementation(
    FRotator NewRotation, bool bResetCamera = false
  ) override;
  //~End of APlayerController interface

  bool bSkipPawnFaceRotation = false;

private:
};

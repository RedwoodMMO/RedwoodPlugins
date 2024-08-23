// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodPlayerController.h"

void ARedwoodPlayerController::ClientSetRotation_Implementation(
  FRotator NewRotation, bool bResetCamera
) {
  SetControlRotation(NewRotation);
  if (GetPawn() != NULL && !bSkipPawnFaceRotation) {
    GetPawn()->FaceRotation(NewRotation, 0.f);
  }
  bSkipPawnFaceRotation = false;
}
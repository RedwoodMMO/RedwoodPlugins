// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodGetBlobAsync.h"

URedwoodGetBlobAsync *URedwoodGetBlobAsync::GetBlob(
  URedwoodServerGameSubsystem *Target, UObject *WorldContextObject, FString Key
) {
  URedwoodGetBlobAsync *Action = NewObject<URedwoodGetBlobAsync>();
  Action->Target = Target;
  Action->Key = Key;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodGetBlobAsync::Activate() {
  Target->GetBlob(
    Key,
    FRedwoodGetBlobOutputDelegate::CreateLambda(
      [this](FRedwoodGetBlobOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

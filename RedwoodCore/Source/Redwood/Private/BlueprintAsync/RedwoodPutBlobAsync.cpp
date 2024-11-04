// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodPutBlobAsync.h"

URedwoodPutBlobAsync *URedwoodPutBlobAsync::PutBlob(
  URedwoodServerGameSubsystem *Target,
  UObject *WorldContextObject,
  FString Key,
  TArray<uint8> Blob
) {
  URedwoodPutBlobAsync *Action = NewObject<URedwoodPutBlobAsync>();
  Action->Target = Target;
  Action->Key = Key;
  Action->Blob = Blob;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodPutBlobAsync::Activate() {
  Target->PutBlob(
    Key, Blob, FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}

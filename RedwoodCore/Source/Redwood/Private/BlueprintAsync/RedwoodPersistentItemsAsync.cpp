// Copyright Incanta Games. All Rights Reserved.

#include "BlueprintAsync/RedwoodPersistentItemsAsync.h"

URedwoodFetchPersistentItemsAsync *
URedwoodFetchPersistentItemsAsync::FetchPersistentItems(
  URedwoodServerGameSubsystem *Target,
  UObject *WorldContextObject,
  FRedwoodPersistentItemsFilter Filter
) {
  URedwoodFetchPersistentItemsAsync *Action =
    NewObject<URedwoodFetchPersistentItemsAsync>();
  Action->Target = Target;
  Action->Filter = Filter;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodFetchPersistentItemsAsync::Activate() {
  Target->FetchPersistentItems(
    Filter,
    FRedwoodPersistentItemsTreeOutputDelegate::CreateLambda(
      [this](FRedwoodPersistentItemsTreeOutput InOutput) {
        // store before broadcasting so the UObject nodes are reachable
        // from a UPROPERTY for the duration of the callback
        Output = InOutput;
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

URedwoodFetchCharacterPersistentItemsAsync *
URedwoodFetchCharacterPersistentItemsAsync::FetchCharacterPersistentItems(
  URedwoodServerGameSubsystem *Target,
  UObject *WorldContextObject,
  FString CharacterId
) {
  URedwoodFetchCharacterPersistentItemsAsync *Action =
    NewObject<URedwoodFetchCharacterPersistentItemsAsync>();
  Action->Target = Target;
  Action->CharacterId = CharacterId;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodFetchCharacterPersistentItemsAsync::Activate() {
  Target->FetchCharacterPersistentItems(
    CharacterId,
    FRedwoodPersistentItemsTreeOutputDelegate::CreateLambda(
      [this](FRedwoodPersistentItemsTreeOutput InOutput) {
        // see URedwoodFetchPersistentItemsAsync::Activate
        Output = InOutput;
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

URedwoodSavePersistentItemsAsync *
URedwoodSavePersistentItemsAsync::SavePersistentItems(
  URedwoodServerGameSubsystem *Target,
  UObject *WorldContextObject,
  TArray<FRedwoodSavePersistentItem> Items
) {
  URedwoodSavePersistentItemsAsync *Action =
    NewObject<URedwoodSavePersistentItemsAsync>();
  Action->Target = Target;
  Action->Items = Items;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodSavePersistentItemsAsync::Activate() {
  Target->SavePersistentItems(
    Items,
    FRedwoodPersistentItemsOutputDelegate::CreateLambda(
      [this](FRedwoodPersistentItemsOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

URedwoodMovePersistentItemsToParentAsync *
URedwoodMovePersistentItemsToParentAsync::MovePersistentItemsToParent(
  URedwoodServerGameSubsystem *Target,
  UObject *WorldContextObject,
  TArray<FString> ItemIds,
  FString NewParentId
) {
  URedwoodMovePersistentItemsToParentAsync *Action =
    NewObject<URedwoodMovePersistentItemsToParentAsync>();
  Action->Target = Target;
  Action->ItemIds = ItemIds;
  Action->NewParentId = NewParentId;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodMovePersistentItemsToParentAsync::Activate() {
  Target->MovePersistentItemsToParent(
    ItemIds,
    NewParentId,
    FRedwoodPersistentItemsOutputDelegate::CreateLambda(
      [this](FRedwoodPersistentItemsOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

URedwoodMovePersistentItemsToCharacterAsync *
URedwoodMovePersistentItemsToCharacterAsync::MovePersistentItemsToCharacter(
  URedwoodServerGameSubsystem *Target,
  UObject *WorldContextObject,
  TArray<FString> ItemIds,
  FString NewOwnerCharacterId
) {
  URedwoodMovePersistentItemsToCharacterAsync *Action =
    NewObject<URedwoodMovePersistentItemsToCharacterAsync>();
  Action->Target = Target;
  Action->ItemIds = ItemIds;
  Action->NewOwnerCharacterId = NewOwnerCharacterId;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodMovePersistentItemsToCharacterAsync::Activate() {
  Target->MovePersistentItemsToCharacter(
    ItemIds,
    NewOwnerCharacterId,
    FRedwoodPersistentItemsOutputDelegate::CreateLambda(
      [this](FRedwoodPersistentItemsOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    )
  );
}

URedwoodMovePersistentItemsToWorldAsync *
URedwoodMovePersistentItemsToWorldAsync::MovePersistentItemsToWorld(
  URedwoodServerGameSubsystem *Target,
  UObject *WorldContextObject,
  TArray<FString> ItemIds,
  FTransform Transform,
  FString ZoneName
) {
  URedwoodMovePersistentItemsToWorldAsync *Action =
    NewObject<URedwoodMovePersistentItemsToWorldAsync>();
  Action->Target = Target;
  Action->ItemIds = ItemIds;
  Action->Transform = Transform;
  Action->ZoneName = ZoneName;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodMovePersistentItemsToWorldAsync::Activate() {
  Target->MovePersistentItemsToWorld(
    ItemIds,
    Transform,
    FRedwoodPersistentItemsOutputDelegate::CreateLambda(
      [this](FRedwoodPersistentItemsOutput Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      }
    ),
    ZoneName
  );
}

URedwoodDeletePersistentItemsAsync *
URedwoodDeletePersistentItemsAsync::DeletePersistentItems(
  URedwoodServerGameSubsystem *Target,
  UObject *WorldContextObject,
  TArray<FString> ItemIds
) {
  URedwoodDeletePersistentItemsAsync *Action =
    NewObject<URedwoodDeletePersistentItemsAsync>();
  Action->Target = Target;
  Action->ItemIds = ItemIds;
  Action->RegisterWithGameInstance(WorldContextObject);

  return Action;
}

void URedwoodDeletePersistentItemsAsync::Activate() {
  Target->DeletePersistentItems(
    ItemIds,
    FRedwoodErrorOutputDelegate::CreateLambda([this](FString Error) {
      OnOutput.Broadcast(Error);
      SetReadyToDestroy();
    })
  );
}

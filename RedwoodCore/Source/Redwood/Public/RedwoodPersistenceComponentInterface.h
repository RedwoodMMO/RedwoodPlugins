// Copyright Incanta LLC. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "UObject/Interface.h"

#include "RedwoodPersistenceComponentInterface.generated.h"

UINTERFACE(MinimalApi)
class URedwoodPersistenceComponentInterface : public UInterface {
  GENERATED_BODY()
};

class REDWOOD_API IRedwoodPersistenceComponentInterface {
  GENERATED_BODY()

public:
  virtual void AddPersistedData(TSharedPtr<FJsonObject> JsonObject) = 0;
};

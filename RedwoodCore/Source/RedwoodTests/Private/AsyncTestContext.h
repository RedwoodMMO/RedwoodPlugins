// Copyright Incanta Games. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

#include "RedwoodTitleInterface.h"

#include "AsyncTestContext.generated.h"

UCLASS()
class UAsyncTestContext : public UObject {
  GENERATED_BODY()

public:
  int32 CurrentTestIndex = -1;
  bool bIsCurrentTestStarting = false;
  bool bIsCurrentTestComplete = false;
  FJsonObject Data;

  void Start() {
    CurrentTestIndex = 0;
    bIsCurrentTestStarting = true;
    bIsCurrentTestComplete = false;
  }
};

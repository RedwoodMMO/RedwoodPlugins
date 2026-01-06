// Copyright Incanta LLC. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Types/RedwoodTypes.h"

#include "RedwoodEOSClientInterface.generated.h"

class URedwoodClientInterface;

UCLASS() class REDWOODEOS_API URedwoodEOSClientInterface : public UObject {
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintCallable, Category = "Redwood|EOS")
  static bool LoginEOS_DevAuthTool(
    FString Endpoint = TEXT("localhost:6300"),
    FString CredentialName = TEXT("Context_1")
  );
  UFUNCTION(BlueprintCallable, Category = "Redwood|EOS")
  static bool LoginEOS_PromptAccountPortal();
  UFUNCTION(BlueprintCallable, Category = "Redwood|EOS")
  static bool LoginEOS_EpicGameStore();

  static void LoginWithEOS(
    URedwoodClientInterface *ClientInterface,
    FRedwoodAuthUpdateDelegate OnUpdate
  );
};

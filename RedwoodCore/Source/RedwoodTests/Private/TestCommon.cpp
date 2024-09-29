// Copyright Incanta Games. All rights reserved.

#include "TestCommon.h"

bool FWaitForEnd::Update() {
  FAutomationTestBase *CurrentTest =
    FAutomationTestFramework::Get().GetCurrentTest();
  bool bIsComplete = CurrentTest->HasAnyErrors();

  if (!bIsComplete) {
    if (Context->CurrentTestIndex == TestIndex) {
      if (Context->bIsCurrentTestStarting) {
        Context->bIsCurrentTestStarting = false;
        StartTime = FPlatformTime::Seconds();
        UE_LOG(
          LogTemp, Warning, TEXT("Deinitializing Redwood in 0.05 seconds")
        );
      } else {
        bIsComplete = true;
      }
    }
  }

  if (bIsComplete) {
    if (FPlatformTime::Seconds() - StartTime > 0.05) {
      Redwood->RemoveFromRoot();
      Context->RemoveFromRoot();
      return true;
    }
  }

  return false;
}

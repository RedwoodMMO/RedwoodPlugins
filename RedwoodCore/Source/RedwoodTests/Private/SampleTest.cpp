// Copyright Incanta Games. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FSampleTest,
  "Redwood.Sample.Test",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
);

bool FSampleTest::RunTest(const FString &Parameters) {
  // args: description, actual, expected
  TestEqual(TEXT("Math works"), 2 + 3, 5);

  return true;
}

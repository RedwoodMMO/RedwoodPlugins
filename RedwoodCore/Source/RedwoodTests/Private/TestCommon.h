// Copyright Incanta Games. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

#include "./AsyncTestContext.h"
#include "RedwoodClientInterface.h"

#define DEFINE_REDWOOD_LATENT_AUTOMATION_COMMAND(CommandName) \
  class CommandName : public IAutomationLatentCommand { \
  public: \
    CommandName( \
      URedwoodClientInterface *InputParam0, \
      UAsyncTestContext *InputParam1, \
      int32 InputParam2 \
    ) : \
      Redwood(InputParam0), \
      Context(InputParam1), TestIndex(InputParam2) { \
      CurrentTest = FAutomationTestFramework::Get().GetCurrentTest(); \
    } \
    virtual ~CommandName() {} \
    virtual bool Update() override; \
    virtual void Initialize(); \
\
  private: \
    FAutomationTestBase *CurrentTest; \
    URedwoodClientInterface *Redwood; \
    UAsyncTestContext *Context; \
    int32 TestIndex; \
  }; \
  bool CommandName::Update() { \
    if (CurrentTest->HasAnyErrors()) { \
      return true; \
    } \
\
    if (Context->CurrentTestIndex > TestIndex) { \
      return true; \
    } \
\
    if (Context->CurrentTestIndex < TestIndex) { \
      return false; \
    } \
\
    if (Context->bIsCurrentTestStarting) { \
      Context->bIsCurrentTestStarting = false; \
      Initialize(); \
    } \
\
    if (Context->bIsCurrentTestComplete) { \
      Context->bIsCurrentTestComplete = false; \
      Context->bIsCurrentTestStarting = true; \
      Context->CurrentTestIndex++; \
      return true; \
    } \
\
    return false; \
  }

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
  FWaitForEnd,
  URedwoodClientInterface *,
  Redwood,
  UAsyncTestContext *,
  Context,
  int32,
  TestIndex
);

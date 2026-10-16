// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RedwoodAngelscriptCallbacks.h"
#include "Types/RedwoodTypes.h"

/**
 * Adapters that turn a single-cast dynamic delegate supplied by an Angelscript
 * caller (see RedwoodAngelscriptCallbacks.h) into the C++ TDelegate that
 * RedwoodCore's methods take.
 *
 * Why this exists: RedwoodCore's subsystem methods take TDelegate callbacks,
 * which UHT cannot expose to script, so each mixin wraps the script callback in
 * a forwarding lambda.
 *
 * Capturing the dynamic delegate by value is deliberate. A dynamic delegate
 * holds a weak reference to its bound object, so if the script object is
 * collected before the backend answers, ExecuteIfBound is a no-op rather than a
 * crash. The lambda may fire more than once for callbacks that report progress
 * (auth and ticketing updates), which a by-value capture also handles.
 */

/** One payload parameter. The common RedwoodCore shape. */
#define RW_FWD(DelegateType, PayloadType, DynamicVar)                          \
  DelegateType::CreateLambda([DynamicVar](const PayloadType &Data) {           \
    DynamicVar.ExecuteIfBound(Data);                                           \
  })

/** No parameters. The FRedwoodDelegate / FRedwoodDynamicCallback pair. */
#define RW_FWD_VOID(DynamicVar)                                                \
  FRedwoodDelegate::CreateLambda([DynamicVar]() {                              \
    DynamicVar.ExecuteIfBound();                                               \
  })

/**
 * Two parameters. RedwoodChat splits the error out of the payload (Error
 * first, then the result) rather than carrying an Error field inside an output
 * struct the way the RedwoodCore calls do, so it needs its own arity.
 */
#define RW_FWD2(DelegateType, FirstType, SecondType, DynamicVar)               \
  DelegateType::CreateLambda(                                                  \
    [DynamicVar](const FirstType &First, const SecondType &Second) {           \
      DynamicVar.ExecuteIfBound(First, Second);                                \
    }                                                                          \
  )

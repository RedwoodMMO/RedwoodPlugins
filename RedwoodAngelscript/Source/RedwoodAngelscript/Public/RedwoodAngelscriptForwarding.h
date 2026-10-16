// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Types/RedwoodTypes.h"

/**
 * Adapters that turn a dynamic delegate supplied by an Angelscript (or
 * Blueprint) caller into the C++ TDelegate that RedwoodCore's methods take.
 *
 * Why this exists: RedwoodCore's subsystem methods take TDelegate callbacks,
 * which UHT cannot expose to script. RedwoodCore already declares a dynamic
 * twin beside every one of those typedefs (FRedwoodAuthUpdateDelegate next to
 * FRedwoodAuthUpdateDynamicDelegate, and so on), which is what the
 * BlueprintAsync nodes consume, so all that is missing is a forwarding lambda.
 *
 * Capturing the dynamic delegate by value is deliberate and matches what the
 * BlueprintAsync nodes do. Dynamic multicast delegates hold weak object
 * references, so a Broadcast after the bound script object is collected is a
 * no-op rather than a crash.
 */

/** One payload parameter. The common RedwoodCore shape. */
#define RW_FWD(DelegateType, PayloadType, DynamicVar)                          \
  DelegateType::CreateLambda([DynamicVar](const PayloadType &Data) {           \
    DynamicVar.Broadcast(Data);                                                \
  })

/** No parameters. The FRedwoodDelegate / FRedwoodDynamicDelegate pair. */
#define RW_FWD_VOID(DynamicVar)                                                \
  FRedwoodDelegate::CreateLambda([DynamicVar]() { DynamicVar.Broadcast(); })

/**
 * Two parameters. RedwoodChat splits the error out of the payload (Error
 * first, then the result) rather than carrying an Error field inside an output
 * struct the way the RedwoodCore calls do, so it needs its own arity.
 */
#define RW_FWD2(DelegateType, FirstType, SecondType, DynamicVar)               \
  DelegateType::CreateLambda(                                                  \
    [DynamicVar](const FirstType &First, const SecondType &Second) {           \
      DynamicVar.Broadcast(First, Second);                                     \
    }                                                                          \
  )

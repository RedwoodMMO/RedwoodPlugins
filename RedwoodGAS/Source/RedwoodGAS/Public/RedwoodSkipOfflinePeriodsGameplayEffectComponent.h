// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "GameplayEffect.h"
#include "GameplayEffectComponent.h"

#include "RedwoodSkipOfflinePeriodsGameplayEffectComponent.generated.h"

/**
 * By adding this component, Redwood will skip executing periodic when the player
 * logs back in for the simulated missed real time. You do not need to add this
 * component if you're already adding URedwoodKeepRemainingTimeGameplayEffectComponent.
 *
 * For example:
 * - By not adding this component, if this effect has a period of 1 minute, the player
 *   logs out, 3.5 minutes pass, and player logs back in, the effect will be executed
 *   3 times immediately.
 * - By not adding this component, if this effect has a period of 1 minute, the player
 *   logs out, 3.5 minutes pass, and player logs back in, the effect will be executed
 *   0 times immediately.
 */
UCLASS(DisplayName = "Redwood: Skip Offline Periods")
class REDWOODGAS_API URedwoodSkipOfflinePeriodsGameplayEffectComponent
  : public UGameplayEffectComponent {
  GENERATED_BODY()
};

// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "GameplayEffect.h"
#include "GameplayEffectComponent.h"

#include "RedwoodKeepRemainingTimeGameplayEffectComponent.generated.h"

/**
 * By adding this component, Redwood will persist the remaining time of this effect as
 * if no time passed. Adding this component also implies skipping offline periods.
 *
 * For example:
 * - By not adding this component, if this effect gets persisted with 1.5 hours left,
 *   player logs out, 2 hours pass, and player logs back in, the effect will no longer
 *   be active.
 * - By adding this component, if this effect gets persisted with 1.5 hours left,
 *   player logs out, 2 hours pass, and player logs back in, the effect will will be
 *   active for another 1.5 hours.
 */
UCLASS(DisplayName = "Redwood: Keep Remaining Time")
class REDWOODGAS_API URedwoodKeepRemainingTimeGameplayEffectComponent
  : public UGameplayEffectComponent {
  GENERATED_BODY()
};

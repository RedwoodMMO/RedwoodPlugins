// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodAbilitySystemComponent.h"
#include "RedwoodCommonGameSubsystem.h"
#include "RedwoodGASModule.h"
#include "RedwoodKeepRemainingTimeGameplayEffectComponent.h"
#include "RedwoodPlayerStateComponent.h"
#include "RedwoodSkipOfflinePeriodsGameplayEffectComponent.h"

#include "GameFramework/PlayerState.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

URedwoodAbilitySystemComponent::URedwoodAbilitySystemComponent(
  const FObjectInitializer &ObjectInitializer
) :
  Super(ObjectInitializer) {
  SetIsReplicatedByDefault(false);
}

void URedwoodAbilitySystemComponent::HandleGameplayEffectApplied(
  UAbilitySystemComponent *ASC,
  const FGameplayEffectSpec &Spec,
  FActiveGameplayEffectHandle ActiveHandle
) {
  MarkDirty();
}

void URedwoodAbilitySystemComponent::HandleGameplayEffectRemoved(
  const FActiveGameplayEffect &ActiveEffect
) {
  MarkDirty();
}

void URedwoodAbilitySystemComponent::OnGiveAbility(
  FGameplayAbilitySpec &AbilitySpec
) {
  Super::OnGiveAbility(AbilitySpec);

  MarkDirty();
}

void URedwoodAbilitySystemComponent::OnRemoveAbility(
  FGameplayAbilitySpec &AbilitySpec
) {
  Super::OnRemoveAbility(AbilitySpec);

  MarkDirty();
}

void URedwoodAbilitySystemComponent::BeginPlay() {
  Super::BeginPlay();

  UWorld *World = GetWorld();

  if (
    IsValid(World) &&
    (
      World->GetNetMode() == ENetMode::NM_DedicatedServer ||
      World->GetNetMode() == ENetMode::NM_ListenServer
    )
  ) {
    OnGameplayEffectAppliedDelegateToSelf.AddUObject(
      this, &URedwoodAbilitySystemComponent::HandleGameplayEffectApplied
    );

    OnAnyGameplayEffectRemovedDelegate().AddUObject(
      this, &URedwoodAbilitySystemComponent::HandleGameplayEffectRemoved
    );

    AActor *Owner = GetOwner();
    APawn *Pawn = Cast<APawn>(Owner);

    if (Pawn) {
      Pawn->ReceiveControllerChangedDelegate.AddUniqueDynamic(
        this, &URedwoodAbilitySystemComponent::OnControllerChanged
      );
      AController *Controller = Pawn->GetController();
      if (IsValid(Controller)) {
        OnControllerChanged(Pawn, nullptr, Controller);
      }
    } else {
      APlayerState *PlayerState = Cast<APlayerState>(Owner);

      if (IsValid(PlayerState)) {
        URedwoodPlayerStateComponent *PlayerStateComponent =
          PlayerState->FindComponentByClass<URedwoodPlayerStateComponent>();

        if (IsValid(PlayerStateComponent)) {
          PlayerStateComponent->OnRedwoodCharacterUpdated.AddUniqueDynamic(
            this,
            &URedwoodAbilitySystemComponent::RedwoodPlayerStateCharacterUpdated
          );
          RedwoodPlayerStateCharacterUpdated();
        } else {
          UE_LOG(
            LogRedwoodGAS,
            Error,
            TEXT(
              "URedwoodAbilitySystemComponent requires the owning APlayerState to have an attached URedwoodPlayerStateComponent"
            )
          );
        }
      } else {
        UE_LOG(
          LogRedwoodGAS,
          Error,
          TEXT(
            "URedwoodAbilitySystemComponent must be used with APawn or APlayerState"
          )
        );
      }
    }
  }
}

bool URedwoodAbilitySystemComponent::AddPersistedData(
  TSharedPtr<FJsonObject> JsonObject, bool bForce
) {
  UWorld *World = GetWorld();

  if (!IsValid(World)) {
    return false;
  }

  if (bForce || !URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld()) || (IsDirty() && (UpdateIntervals <= 0 || UpdateIntervalCounter % UpdateIntervals == 0))) {
    UpdateIntervalCounter = 0;
    TSharedPtr<FJsonObject> AbilitySystemObject = SerializeASC();

    bool bReturnTrue = false;
    if (AbilitySystemObject.IsValid()) {
      JsonObject->SetObjectField(TEXT("abilitySystem"), AbilitySystemObject);
      bReturnTrue = true;
    }

    bDirty = false;

    return bReturnTrue;
  } else if (URedwoodCommonGameSubsystem::ShouldUseBackend(GetWorld()) && UpdateIntervals > 0) {
    UpdateIntervalCounter++;

    return false;
  }
}

void URedwoodAbilitySystemComponent::OnControllerChanged(
  APawn *Pawn, AController *OldController, AController *NewController
) {
  if (IsValid(NewController)) {
    URedwoodPlayerStateComponent *PlayerStateComponent =
      NewController->PlayerState
        ->FindComponentByClass<URedwoodPlayerStateComponent>();
    if (IsValid(PlayerStateComponent)) {
      PlayerStateComponent->OnRedwoodCharacterUpdated.AddUniqueDynamic(
        this,
        &URedwoodAbilitySystemComponent::RedwoodPlayerStateCharacterUpdated
      );
      RedwoodPlayerStateCharacterUpdated();
    }
  }
}

void URedwoodAbilitySystemComponent::RedwoodPlayerStateCharacterUpdated() {
  APawn *Pawn = Cast<APawn>(GetOwner());
  AController *Controller = IsValid(Pawn) ? Pawn->GetController() : nullptr;
  APlayerState *PlayerState = IsValid(Controller)
    ? Cast<APlayerState>(Controller->PlayerState)
    : Cast<APlayerState>(GetOwner());

  URedwoodPlayerStateComponent *PlayerStateComponent = IsValid(PlayerState)
    ? PlayerState->FindComponentByClass<URedwoodPlayerStateComponent>()
    : nullptr;

  if (IsValid(PlayerStateComponent)) {
    FRedwoodCharacterBackend RedwoodCharacterBackend =
      PlayerStateComponent->RedwoodCharacter;

    if (IsValid(RedwoodCharacterBackend.AbilitySystem)) {
      DeserializeASC(RedwoodCharacterBackend.AbilitySystem->GetRootObject());
    }
  }
}

TSharedPtr<FJsonObject> URedwoodAbilitySystemComponent::SerializeASC() {
  UWorld *World = GetWorld();

  if (!IsValid(World)) {
    return nullptr;
  }

  double WorldTimeSeconds = World->GetTimeSeconds();

  TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

  double CurrentTimestampMs = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
  JsonObject->SetNumberField(TEXT("timestampMs"), CurrentTimestampMs);

  // -----------
  // --EFFECTS--
  // -----------
  const FActiveGameplayEffectsContainer &OutActiveEffectsContainer =
    GetActiveGameplayEffects();
  TArray<FActiveGameplayEffectHandle> OutActiveEffectHandles =
    OutActiveEffectsContainer.GetAllActiveEffectHandles();

  TArray<TSharedPtr<FJsonValue>> EffectObjects;
  for (const FActiveGameplayEffectHandle &ActiveEffectHandle :
       OutActiveEffectHandles) {
    // Serialize each spec
    const FActiveGameplayEffect *ActiveEffect =
      OutActiveEffectsContainer.GetActiveGameplayEffect(ActiveEffectHandle);

    FTopLevelAssetPath EffectClassName =
      ActiveEffect->Spec.Def->GetClass()->GetClassPathName();

    bool bFoundClass = false;
    for (TSubclassOf<UGameplayEffect> Subclass : EffectInclusionArray) {
      if (Subclass != nullptr && EffectClassName == Subclass->GetClassPathName()) {
        bFoundClass = true;
      }
    }

    if ((EffectInclusionMode == ERedwoodASCInclusionMode::Blacklist && bFoundClass) || (EffectInclusionMode == ERedwoodASCInclusionMode::Whitelist && !bFoundClass)) {
      continue;
    }

    float Level = ActiveEffect->Spec.GetLevel();
    float TimeLeft = ActiveEffect->GetTimeRemaining(WorldTimeSeconds);

    TSharedPtr<FJsonObject> EffectObject = MakeShareable(new FJsonObject);
    EffectObject->SetStringField(TEXT("class"), EffectClassName.ToString());
    EffectObject->SetNumberField(TEXT("level"), Level);
    EffectObject->SetNumberField(TEXT("timeLeft"), TimeLeft);

    EffectObjects.Add(MakeShareable(new FJsonValueObject(EffectObject)));
  }
  JsonObject->SetArrayField(TEXT("effects"), EffectObjects);

  // -------------
  // --ABILITIES--
  // -------------
  TArray<FGameplayAbilitySpecHandle> OutAbilityHandles;
  GetAllAbilities(OutAbilityHandles);

  TArray<TSharedPtr<FJsonValue>> AbilitiesArray;
  for (const FGameplayAbilitySpecHandle &AbilityHandle : OutAbilityHandles) {
    FGameplayAbilitySpec *AbilitySpec =
      FindAbilitySpecFromHandle(AbilityHandle, EConsiderPending::PendingAdd);

    if (AbilitySpec) {
      FTopLevelAssetPath AbilityClassName =
        AbilitySpec->Ability->GetClass()->GetClassPathName();
      int32 Level = AbilitySpec->Level;
      int32 InputID = AbilitySpec->InputID;

      bool bFoundClass = false;
      for (TSubclassOf<UGameplayAbility> Subclass : AbilityInclusionArray) {
        if (Subclass != nullptr && AbilityClassName == Subclass->GetClassPathName()) {
          bFoundClass = true;
        }
      }

      if ((AbilityInclusionMode == ERedwoodASCInclusionMode::Blacklist && bFoundClass) || (AbilityInclusionMode == ERedwoodASCInclusionMode::Whitelist && !bFoundClass)) {
        continue;
      }

      TSharedPtr<FJsonObject> AbilityObject = MakeShareable(new FJsonObject);
      AbilityObject->SetStringField(TEXT("class"), AbilityClassName.ToString());
      AbilityObject->SetNumberField(TEXT("level"), Level);
      AbilityObject->SetNumberField(TEXT("inputId"), InputID);

      AbilitiesArray.Add(MakeShareable(new FJsonValueObject(AbilityObject)));
    }
  }
  JsonObject->SetArrayField(TEXT("abilities"), AbilitiesArray);

  // --------------
  // --ATTRIBUTES--
  // --------------
  TArray<TSharedPtr<FJsonValue>> AttributeSetsObject;
  for (const UAttributeSet *Set : GetSpawnedAttributes()) {
    if (!Set) {
      continue;
    }

    TArray<FGameplayAttribute> OutAttributes;
    UAttributeSet::GetAttributesFromSetClass(Set->GetClass(), OutAttributes);

    TSet<uint32> AttributeInclusionHashes;
    for (FGameplayAttribute Attribute : AttributeInclusionArray) {
      if (Attribute.IsValid()) {
        AttributeInclusionHashes.Add(GetTypeHash(Attribute));
      }
    }

    TArray<TSharedPtr<FJsonValue>> AttributesObject;
    for (const FGameplayAttribute &Attribute : OutAttributes) {
      bool bInclusionArrayContainsAttribute =
        AttributeInclusionHashes.Contains(GetTypeHash(Attribute));

      if ((AttributeInclusionMode == ERedwoodASCInclusionMode::Blacklist && bInclusionArrayContainsAttribute) || (AttributeInclusionMode == ERedwoodASCInclusionMode::Whitelist && !bInclusionArrayContainsAttribute)) {
        continue;
      }

      const FGameplayAttributeData *AttributeData =
        Attribute.GetGameplayAttributeData(Set);

      bool bHasBase = false;
      float Base = 0;
      float Current = Attribute.GetNumericValue(Set);
      if (AttributeData) {
        bHasBase = true;
        Base = AttributeData->GetBaseValue();
        Current = AttributeData->GetCurrentValue();
      }

      TSharedPtr<FJsonObject> AttributeObject = MakeShareable(new FJsonObject);
      AttributeObject->SetStringField(TEXT("name"), Attribute.GetName());
      if (bHasBase) {
        AttributeObject->SetNumberField(TEXT("base"), Base);
      } else {
        TSharedPtr<FJsonValue> NullValue = MakeShareable(new FJsonValueNull);
        AttributeObject->SetField(TEXT("base"), NullValue);
      }
      AttributeObject->SetNumberField(TEXT("current"), Current);

      AttributesObject.Add(MakeShareable(new FJsonValueObject(AttributeObject))
      );
    }

    if (AttributesObject.Num() > 0) {
      FTopLevelAssetPath AttributeSetName = Set->GetClass()->GetClassPathName();

      TSharedPtr<FJsonObject> AttributeSetObject =
        MakeShareable(new FJsonObject);
      AttributeSetObject->SetStringField(
        TEXT("name"), AttributeSetName.ToString()
      );
      AttributeSetObject->SetArrayField(TEXT("attributes"), AttributesObject);

      AttributeSetsObject.Add(
        MakeShareable(new FJsonValueObject(AttributeSetObject))
      );
    }
  }
  JsonObject->SetArrayField(TEXT("attributeSets"), AttributeSetsObject);

  return JsonObject;
}

void URedwoodAbilitySystemComponent::DeserializeASC(TSharedPtr<FJsonObject> Data
) {
  if (!Data.IsValid()) {
    return;
  }

  UWorld *World = GetWorld();

  if (!World) {
    return;
  }

  if (
    !Data->HasField(TEXT("timestampMs")) ||
    !Data->HasField(TEXT("effects")) ||
    !Data->HasField(TEXT("abilities")) ||
    !Data->HasField(TEXT("attributeSets"))
  ) {
    return;
  }

  double TimestampMs = Data->GetNumberField(TEXT("timestampMs"));
  double CurrentTimestampMs = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
  double TimeSinceSaveSec = (CurrentTimestampMs - TimestampMs) / 1000.0;

  TMap<FActiveGameplayEffectHandle, int32> EffectsToExecutePeriods;
  const TArray<TSharedPtr<FJsonValue>> *EffectsArray;
  if (Data->TryGetArrayField(TEXT("effects"), EffectsArray)) {
    for (const TSharedPtr<FJsonValue> &EffectValue : *EffectsArray) {
      const TSharedPtr<FJsonObject> &EffectObject = EffectValue->AsObject();
      if (EffectObject) {
        FString ClassName = EffectObject->GetStringField(TEXT("class"));
        float Level = EffectObject->GetNumberField(TEXT("level"));
        float TimeLeft = EffectObject->GetNumberField(TEXT("timeLeft"));

        TSubclassOf<UGameplayEffect> EffectClass =
          LoadClass<UGameplayEffect>(nullptr, *ClassName);

        UGameplayEffect *Effect =
          EffectClass->GetDefaultObject<UGameplayEffect>();

        bool bShouldPreserveTimeLeft =
          Effect->FindComponent(
            URedwoodKeepRemainingTimeGameplayEffectComponent::StaticClass()
          ) != nullptr;

        bool bShouldSkipOfflinePeriods = bShouldPreserveTimeLeft ||
          Effect->FindComponent(
            URedwoodSkipOfflinePeriodsGameplayEffectComponent::StaticClass()
          ) != nullptr;

        Effect->bExecutePeriodicEffectOnApplication = false;

        FGameplayEffectContext *EffectContext = new FGameplayEffectContext();
        // optionally call things like AddInstigator, SetAbility, See GameplayEffectTypes.h

        FGameplayEffectSpec EffectSpec(
          Effect, FGameplayEffectContextHandle(EffectContext), Level
        );

        FPredictionKey FakePredictionKey;
        bool bFoundExistingStackableGE = false;
        FActiveGameplayEffect *ActiveGameplayEffect =
          ActiveGameplayEffects.ApplyGameplayEffectSpec(
            EffectSpec, FakePredictionKey, bFoundExistingStackableGE
          );

        if (!ActiveGameplayEffect) {
          UE_LOG(
            LogRedwoodGAS,
            Error,
            TEXT(
              "URedwoodAbilitySystemComponent::DeserializeASC: Could not apply gameplay effect spec for class %s"
            ),
            *ClassName
          );
          continue;
        }

        // For some reason the pointer returned above doesn't seem to actually
        // modify the effect, so look it up again to get the right pointer
        ActiveGameplayEffect = ActiveGameplayEffects.GetActiveGameplayEffect(
          ActiveGameplayEffect->Handle
        );

        if (EffectSpec.GetDuration() > 0) {
          double AdjustedTimeLeft = bShouldPreserveTimeLeft ? TimeLeft
            : TimeLeft > TimeSinceSaveSec ? TimeLeft - TimeSinceSaveSec
                                          : 0;
          float StartTime = TimeLeft < 0 ? World->GetTimeSeconds()
                                         : World->GetTimeSeconds() +
              AdjustedTimeLeft - EffectSpec.GetDuration();

          ActiveGameplayEffect->StartWorldTime = StartTime;
          ActiveGameplayEffect->StartServerWorldTime = StartTime;
          ActiveGameplayEffect->CachedStartServerWorldTime = StartTime;

          FTimerManager &TimerManager = World->GetTimerManager();
          FTimerDelegate Delegate = FTimerDelegate::CreateUObject(
            this,
            &URedwoodAbilitySystemComponent::CheckDurationExpired,
            ActiveGameplayEffect->Handle
          );
          TimerManager.SetTimer(
            ActiveGameplayEffect->DurationHandle,
            Delegate,
            AdjustedTimeLeft,
            false
          );
          if (!ensureMsgf(
                ActiveGameplayEffect->DurationHandle.IsValid(),
                TEXT(
                  "Invalid Duration Handle after attempting to set duration for GE (%s) @ %.2f"
                ),
                *ActiveGameplayEffect->GetDebugString(),
                AdjustedTimeLeft
              )) {
            // Force this off next frame
            TimerManager.SetTimerForNextTick(Delegate);
          }
        }

        if (!bShouldSkipOfflinePeriods && EffectSpec.GetPeriod() > 0) {
          float ActiveTimeSinceSave =
            TimeLeft > TimeSinceSaveSec ? TimeSinceSaveSec : TimeLeft;

          int32 NumPeriodsToExecute =
            FMath::FloorToInt(ActiveTimeSinceSave / EffectSpec.GetPeriod());

          if (NumPeriodsToExecute > 0) {
            EffectsToExecutePeriods.Add(
              ActiveGameplayEffect->Handle, NumPeriodsToExecute
            );
          }
        }
      }
    }
  }

  const TArray<TSharedPtr<FJsonValue>> *AbilitiesArray;
  if (Data->TryGetArrayField(TEXT("abilities"), AbilitiesArray)) {
    for (const TSharedPtr<FJsonValue> &AbilityValue : *AbilitiesArray) {
      const TSharedPtr<FJsonObject> &AbilityObject = AbilityValue->AsObject();
      if (AbilityObject) {
        FString ClassName = AbilityObject->GetStringField(TEXT("class"));
        int32 Level =
          FMath::FloorToInt(AbilityObject->GetNumberField(TEXT("level")));
        int32 InputID =
          FMath::FloorToInt(AbilityObject->GetNumberField(TEXT("inputId")));

        TSubclassOf<UGameplayAbility> AbilityClass =
          LoadClass<UGameplayAbility>(nullptr, *ClassName);

        FGameplayAbilitySpec NewSpec(AbilityClass, Level, InputID, this);

        GiveAbility(NewSpec);
      }
    }
  }

  const TArray<TSharedPtr<FJsonValue>> *AttributeSetsArray;
  if (Data->TryGetArrayField(TEXT("attributeSets"), AttributeSetsArray)) {
    for (const TSharedPtr<FJsonValue> &AttributeSetValue :
         *AttributeSetsArray) {
      const TSharedPtr<FJsonObject> &AttributeSetObject =
        AttributeSetValue->AsObject();
      if (AttributeSetObject) {
        FString AttributeSetName =
          AttributeSetObject->GetStringField(TEXT("name"));

        const TArray<UAttributeSet *> &AttributeSets = GetSpawnedAttributes();

        UAttributeSet *AttributeSet = nullptr;

        for (UAttributeSet *SpawnedAttributeSet : AttributeSets) {
          if (SpawnedAttributeSet->GetClass()->GetClassPathName().ToString() == AttributeSetName) {
            AttributeSet = SpawnedAttributeSet;
            break;
          }
        }

        if (!AttributeSet) {
          UE_LOG(
            LogRedwoodGAS,
            Error,
            TEXT(
              "URedwoodAbilitySystemComponent::DeserializeASC: Could not find attribute set for class %s"
            ),
            *AttributeSetName
          );
          continue;
        }

        TArray<FGameplayAttribute> OutAttributes;
        UAttributeSet::GetAttributesFromSetClass(
          AttributeSet->GetClass(), OutAttributes
        );

        TArray<TSharedPtr<FJsonValue>> AttributesArray =
          AttributeSetObject->GetArrayField(TEXT("attributes"));

        for (const FGameplayAttribute &Attribute : OutAttributes) {
          for (const TSharedPtr<FJsonValue> &AttributeValue : AttributesArray) {
            const TSharedPtr<FJsonObject> &AttributeObject =
              AttributeValue->AsObject();
            if (AttributeObject) {
              FString AttributeName =
                AttributeObject->GetStringField(TEXT("name"));

              if (AttributeName != Attribute.GetName()) {
                continue;
              }

              FGameplayAttributeData *AttributeData =
                Attribute.GetGameplayAttributeData(AttributeSet);

              float Base = -1;
              AttributeObject->TryGetNumberField(TEXT("base"), Base);
              float Current = AttributeObject->GetNumberField(TEXT("current"));

              if (AttributeData) {
                AttributeData->SetBaseValue(Base);
                AttributeData->SetCurrentValue(Current);
              } else {
                Attribute.SetNumericValueChecked(Current, AttributeSet);
              }

              break;
            }
          }
        }
      }
    }
  }

  for (const TPair<FActiveGameplayEffectHandle, int32> &Pair :
       EffectsToExecutePeriods) {
    FActiveGameplayEffectHandle Handle = Pair.Key;
    int32 NumPeriodsToExecute = Pair.Value;

    for (int32 i = 0; i < NumPeriodsToExecute; i++) {
      ActiveGameplayEffects.ExecutePeriodicGameplayEffect(Handle);
    }
  }
}

// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodAbilitySystemComponent.h"
#include "RedwoodCommonGameSubsystem.h"
#include "RedwoodGASModule.h"
#include "RedwoodPlayerState.h"

#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

URedwoodAbilitySystemComponent::URedwoodAbilitySystemComponent(
  const FObjectInitializer &ObjectInitializer
) :
  Super(ObjectInitializer) {
  SetIsReplicatedByDefault(false);
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
      ARedwoodPlayerState *RedwoodPlayerState =
        Cast<ARedwoodPlayerState>(Owner);

      if (IsValid(RedwoodPlayerState)) {
        RedwoodPlayerState->OnRedwoodCharacterUpdated.AddUniqueDynamic(
          this,
          &URedwoodAbilitySystemComponent::RedwoodPlayerStateCharacterUpdated
        );
        RedwoodPlayerStateCharacterUpdated();
      } else {
        UE_LOG(
          LogRedwoodGAS,
          Error,
          TEXT(
            "URedwoodAbilitySystemComponent must be used with APawn or ARedwoodPlayerState"
          )
        );
      }
    }
  }
}

void URedwoodAbilitySystemComponent::OnControllerChanged(
  APawn *Pawn, AController *OldController, AController *NewController
) {
  if (IsValid(NewController)) {
    TObjectPtr<ARedwoodPlayerState> RedwoodPlayerState =
      Cast<ARedwoodPlayerState>(NewController->PlayerState);
    if (RedwoodPlayerState) {
      RedwoodPlayerState->OnRedwoodCharacterUpdated.AddUniqueDynamic(
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
  ARedwoodPlayerState *RedwoodPlayerState = IsValid(Controller)
    ? Cast<ARedwoodPlayerState>(Controller->PlayerState)
    : Cast<ARedwoodPlayerState>(GetOwner());
  if (IsValid(RedwoodPlayerState)) {
    FRedwoodCharacterBackend RedwoodCharacterBackend =
      RedwoodPlayerState->RedwoodCharacter;

    // bool bShouldMarkDirty = URedwoodCommonGameSubsystem::DeserializeBackendData(
    //   bStoreDataInActor ? (UObject *)Pawn : (UObject *)this,
    //   RedwoodCharacterBackend.AbilitySystem,
    //   *AbilitySystemVariableName,
    //   LatestAbilitySystemSchemaVersion
    // );

    // if (bShouldMarkDirty) {
    //   MarkDirty();
    // }

    // TODO serialize ASC
    // TArray<FGameplayAttribute> Attributes;
    // ASC->GetAllAttributes(Attributes);
  }
}

void URedwoodAbilitySystemComponent::DoSerialize() {
  TSharedPtr<FJsonObject> JsonObject = SerializeASC();

  if (!JsonObject.IsValid()) {
    return;
  }

  // save to disk

  FString SavePath =
    FPaths::ProjectSavedDir() / TEXT("Persistence") / TEXT("Characters");
  FPaths::NormalizeFilename(SavePath);
  FString FileName = SavePath / TEXT("gas.json");

  JsonObject->SetNumberField(
    TEXT("timestampMs"), FDateTime::Now().ToUnixTimestamp() * 1000
  );

  FString OutputString;
  TSharedRef<TJsonWriter<TCHAR>> JsonWriter =
    TJsonWriterFactory<TCHAR>::Create(&OutputString);
  FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

  FFileHelper::SaveStringToFile(OutputString, *FileName);
}

void URedwoodAbilitySystemComponent::DoDeserialize() {
  FString SavePath =
    FPaths::ProjectSavedDir() / TEXT("Persistence") / TEXT("Characters");
  FPaths::NormalizeFilename(SavePath);
  FString FileName = SavePath / TEXT("gas.json");

  FString FileContents;
  if (!FFileHelper::LoadFileToString(FileContents, *FileName)) {
    return;
  }

  TSharedPtr<FJsonObject> JsonObject;
  TSharedRef<TJsonReader<>> JsonReader =
    TJsonReaderFactory<>::Create(FileContents);

  if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid()) {
    return;
  }

  double TimestampMs = JsonObject->GetNumberField(TEXT("timestampMs"));
  double CurrentTimestampMs = FDateTime::Now().ToUnixTimestamp() * 1000;
  double TimeSinceSaveSec = (CurrentTimestampMs - TimestampMs) / 1000.0;
  JsonObject->SetNumberField(TEXT("timeSinceSaveSec"), TimeSinceSaveSec);

  DeserializeASC(JsonObject);
}

TSharedPtr<FJsonObject> URedwoodAbilitySystemComponent::SerializeASC() {
  UWorld *World = GetWorld();

  if (!IsValid(World)) {
    return nullptr;
  }

  double WorldTimeSeconds = World->GetTimeSeconds();

  TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

  // --------
  // --TAGS--
  // --------
  const FGameplayTagContainer &OutOwnedGameplayTagsContainer =
    GetOwnedGameplayTags();
  const TArray<FGameplayTag> &OutOwnedGameplayTags =
    OutOwnedGameplayTagsContainer.GetGameplayTagArray();
  TArray<TSharedPtr<FJsonValue>> OutOwnedGameplayTagsJson;
  for (const FGameplayTag &Tag : OutOwnedGameplayTags) {
    OutOwnedGameplayTagsJson.Add(
      MakeShareable(new FJsonValueString(Tag.ToString()))
    );
  }

  // const FGameplayTagContainer &OutBlockedAbilityTagsContainer =
  //   GetBlockedAbilityTags();
  // const TArray<FGameplayTag> &OutBlockedAbilityTags =
  //   OutBlockedAbilityTagsContainer.GetGameplayTagArray();
  // TArray<TSharedPtr<FJsonValue>> OutBlockedAbilityTagsJson;
  // for (const FGameplayTag &Tag : OutBlockedAbilityTags) {
  //   OutBlockedAbilityTagsJson.Add(
  //     MakeShareable(new FJsonValueString(Tag.ToString()))
  //   );
  // }

  TSharedPtr<FJsonObject> TagsObject = MakeShareable(new FJsonObject);
  TagsObject->SetField(
    TEXT("owned"), MakeShareable(new FJsonValueArray(OutOwnedGameplayTagsJson))
  );
  // TagsObject->SetField(
  //   TEXT("blockedAbility"),
  //   MakeShareable(new FJsonValueArray(OutBlockedAbilityTagsJson))
  // );
  JsonObject->SetObjectField(TEXT("tags"), TagsObject);

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

    TArray<TSharedPtr<FJsonValue>> AttributesObject;
    for (const FGameplayAttribute &Attribute : OutAttributes) {
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
    FTopLevelAssetPath AttributeSetName = Set->GetClass()->GetClassPathName();

    TSharedPtr<FJsonObject> AttributeSetObject = MakeShareable(new FJsonObject);
    AttributeSetObject->SetStringField(
      TEXT("name"), AttributeSetName.ToString()
    );
    AttributeSetObject->SetArrayField(TEXT("attributes"), AttributesObject);

    AttributeSetsObject.Add(
      MakeShareable(new FJsonValueObject(AttributeSetObject))
    );
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

  double TimeSinceSaveSec = Data->GetNumberField(TEXT("timeSinceSaveSec"));

  const TSharedPtr<FJsonObject> &TagsObject =
    Data->GetObjectField(TEXT("tags"));

  TArray<FString> OwnedTags;
  TagsObject->TryGetStringArrayField(TEXT("owned"), OwnedTags);
  for (const FString Tag : OwnedTags) {
    FGameplayTag GameplayTag = FGameplayTag::RequestGameplayTag(FName(*Tag));
    UpdateTagMap(GameplayTag, 1);
  }

  // TArray<FString> BlockedAbilityTags;
  // TagsObject.TryGetStringArrayField(TEXT("blockedAbility"), OwnedTags);

  TMap<FActiveGameplayEffectHandle, int32> EffectsToExecutePeriods;
  const TArray<TSharedPtr<FJsonValue>> &EffectsArray =
    Data->GetArrayField(TEXT("effects"));
  for (const TSharedPtr<FJsonValue> &EffectValue : EffectsArray) {
    const TSharedPtr<FJsonObject> &EffectObject = EffectValue->AsObject();
    if (EffectObject) {
      FString ClassName = EffectObject->GetStringField(TEXT("class"));
      float Level = EffectObject->GetNumberField(TEXT("level"));
      float TimeLeft = EffectObject->GetNumberField(TEXT("timeLeft"));

      TSubclassOf<UGameplayEffect> EffectClass =
        LoadClass<UGameplayEffect>(nullptr, *ClassName);

      UGameplayEffect *Effect =
        EffectClass->GetDefaultObject<UGameplayEffect>();

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
        double AdjustedTimeLeft =
          TimeLeft > TimeSinceSaveSec ? TimeLeft - TimeSinceSaveSec : 0;
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

      if (EffectSpec.GetPeriod() > 0) {
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

  const TArray<TSharedPtr<FJsonValue>> &AbilitiesArray =
    Data->GetArrayField(TEXT("abilities"));
  for (const TSharedPtr<FJsonValue> &AbilityValue : AbilitiesArray) {
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

  TArray<TSharedPtr<FJsonValue>> AttributeSetsArray =
    Data->GetArrayField(TEXT("attributeSets"));
  for (const TSharedPtr<FJsonValue> &AttributeSetValue : AttributeSetsArray) {
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

  for (const TPair<FActiveGameplayEffectHandle, int32> &Pair :
       EffectsToExecutePeriods) {
    FActiveGameplayEffectHandle Handle = Pair.Key;
    int32 NumPeriodsToExecute = Pair.Value;

    for (int32 i = 0; i < NumPeriodsToExecute; i++) {
      ActiveGameplayEffects.ExecutePeriodicGameplayEffect(Handle);
    }
  }
}

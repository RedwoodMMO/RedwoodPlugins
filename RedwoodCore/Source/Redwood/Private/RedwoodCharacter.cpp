// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodCharacter.h"
#include "RedwoodPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "SIOJsonObject.h"

ARedwoodCharacter::ARedwoodCharacter(const FObjectInitializer &ObjectInitializer
) :
  Super(ObjectInitializer) {
  SetReplicates(true);
  SetReplicateMovement(true);
}

void ARedwoodCharacter::GetLifetimeReplicatedProps(
  TArray<FLifetimeProperty> &OutLifetimeProps
) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(ARedwoodCharacter, RedwoodPlayerId);
  DOREPLIFETIME(ARedwoodCharacter, RedwoodCharacterId);
  DOREPLIFETIME(ARedwoodCharacter, RedwoodCharacterName);
}

void ARedwoodCharacter::BeginPlay() {
  Super::BeginPlay();
}

void ARedwoodCharacter::PossessedBy(AController *NewController) {
  Super::PossessedBy(NewController);

  ARedwoodPlayerState *RedwoodPlayerState =
    Cast<ARedwoodPlayerState>(GetPlayerState());
  if (RedwoodPlayerState) {
    RedwoodPlayerState->OnRedwoodCharacterUpdated.AddUniqueDynamic(
      this, &ARedwoodCharacter::RedwoodPlayerStateCharacterUpdated
    );
    RedwoodPlayerStateCharacterUpdated();
  }
}

void ARedwoodCharacter::DeserializeBackendData(
  USIOJsonObject *SIOJsonObject, FString VariableName, int32 LatestSchemaVersion
) {
  if (SIOJsonObject) {
    FProperty *Prop = this->GetClass()->FindPropertyByName(*VariableName);
    if (Prop) {
      FStructProperty *StructProp = CastField<FStructProperty>(Prop);
      if (StructProp) {
        TSharedPtr<FJsonObject> JsonObject = SIOJsonObject->GetRootObject();

        UStruct *StructDefinition = StructProp->Struct;

        int32 SchemaVersion = 0;
        if (!JsonObject->TryGetNumberField(
              TEXT("schemaVersion"), SchemaVersion
            )) {
          UE_LOG(
            LogRedwood,
            Error,
            TEXT(
              "schemaVersion not found in Redwood backend field for %s, did you add one to your struct? Not updating %s."
            ),
            *VariableName,
            *VariableName
          );
        }

        void *StructPtr = FMemory::Malloc(StructDefinition->GetStructureSize());

        bool bSuccess = StructDefinition == nullptr
          ? false
          : USIOJConvert::JsonObjectToUStruct(
              JsonObject, StructDefinition, StructPtr, true
            );

        if (bSuccess) {
          while (SchemaVersion < LatestSchemaVersion) {
            // call a migration functions
            FString MigrationFunctionName = FString::Printf(
              TEXT("%s_Migrate_v%d"), *VariableName, SchemaVersion
            );
            UFunction *MigrationFunction =
              this->GetClass()->FindFunctionByName(*MigrationFunctionName);

            if (MigrationFunction) {
              // Ensure the function is valid and has the correct signature
              if (!MigrationFunction->IsValidLowLevel() || MigrationFunction->NumParms != 3 || !MigrationFunction->ReturnValueOffset)
                  {
                UE_LOG(
                  LogRedwood,
                  Error,
                  TEXT(
                    "Migration function %s in %s has an invalid signature, skipping update."
                  ),
                  *MigrationFunctionName,
                  *GetName()
                );

                break;
              } else {
                // Allocate memory for the parameters
                void *Params = FMemory::Malloc(MigrationFunction->ParmsSize);
                FMemory::Memzero(Params, MigrationFunction->ParmsSize);

                FProperty *FunctionStructProp = MigrationFunction->PropertyLink;
                FProperty *FunctionObjectProp =
                  FunctionStructProp->PropertyLinkNext;

                // Set the input parameters
                void *StructParam =
                  FunctionStructProp->ContainerPtrToValuePtr<void *>(Params);
                FMemory::Memcpy(
                  StructParam, StructPtr, StructDefinition->GetStructureSize()
                );

                // ObjectParam is a pointer to a USIOJsonObject pointer
                USIOJsonObject **ObjectParam =
                  FunctionObjectProp->ContainerPtrToValuePtr<USIOJsonObject *>(
                    Params
                  );
                *ObjectParam = SIOJsonObject;

                // Call the function
                this->ProcessEvent(MigrationFunction, Params);

                // Retrieve the return value
                void *ReturnValue =
                  (void
                     *)((SIZE_T)Params + MigrationFunction->ReturnValueOffset);

                // Copy the return value
                FMemory::Free(StructPtr);
                StructPtr =
                  FMemory::Malloc(StructDefinition->GetStructureSize());
                FMemory::Memcpy(
                  StructPtr, ReturnValue, StructDefinition->GetStructureSize()
                );

                // Clean up
                FMemory::Free(Params);
              }
            }

            SchemaVersion++;
          }

          if (SchemaVersion == LatestSchemaVersion) {
            StructProp->CopySingleValue(
              StructProp->ContainerPtrToValuePtr<void>(this), StructPtr
            );
          }

          FMemory::Free(StructPtr);
        } else {
          UE_LOG(
            LogRedwood,
            Error,
            TEXT("Failed to convert JSON object for %s, schemaVersion %d"),
            *GetName(),
            SchemaVersion
          );
        }
      } else {
        UE_LOG(
          LogRedwood,
          Error,
          TEXT("%s variable in %s is not a struct"),
          *VariableName,
          *GetName()
        );
      }
    } else {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT("%s variable not found in %s"),
        *VariableName,
        *GetName()
      );
    }
  }
}

USIOJsonObject *ARedwoodCharacter::SerializeBackendData(FString VariableName) {
  FProperty *Prop = this->GetClass()->FindPropertyByName(*VariableName);
  if (Prop) {
    FStructProperty *StructProp = CastField<FStructProperty>(Prop);
    if (StructProp) {
      UStruct *StructDefinition = StructProp->Struct;

      void *StructPtr = StructProp->ContainerPtrToValuePtr<void>(this);

      TSharedPtr<FJsonObject> JsonObject =
        USIOJConvert::ToJsonObject(StructDefinition, StructPtr, true);

      USIOJsonObject *SIOJsonObject = NewObject<USIOJsonObject>();
      SIOJsonObject->SetRootObject(JsonObject);

      return SIOJsonObject;
    } else {
      UE_LOG(
        LogRedwood,
        Error,
        TEXT("%s variable in %s is not a struct"),
        *VariableName,
        *GetName()
      );
    }
  } else {
    UE_LOG(
      LogRedwood,
      Error,
      TEXT("%s variable not found in %s"),
      *VariableName,
      *GetName()
    );
  }

  return nullptr;
}

void ARedwoodCharacter::RedwoodPlayerStateCharacterUpdated() {
  ARedwoodPlayerState *RedwoodPlayerState =
    Cast<ARedwoodPlayerState>(GetPlayerState());
  if (RedwoodPlayerState) {
    FRedwoodCharacterBackend RedwoodCharacterBackend =
      RedwoodPlayerState->RedwoodCharacter;

    RedwoodPlayerId = RedwoodCharacterBackend.PlayerId;
    RedwoodCharacterId = RedwoodCharacterBackend.Id;

    DeserializeBackendData(
      RedwoodCharacterBackend.Metadata,
      TEXT("Metadata"),
      LatestMetadataSchemaVersion
    );

    DeserializeBackendData(
      RedwoodCharacterBackend.EquippedInventory,
      TEXT("EquippedInventory"),
      LatestEquippedSchemaVersion
    );

    DeserializeBackendData(
      RedwoodCharacterBackend.NonequippedInventory,
      TEXT("NonequippedInventory"),
      LatestNonequippedSchemaVersion
    );

    DeserializeBackendData(
      RedwoodCharacterBackend.Data, TEXT("Data"), LatestDataSchemaVersion
    );

    OnRedwoodCharacterUpdated();
  }
}

void ARedwoodCharacter::OnRedwoodCharacterUpdated_Implementation() {
  //
}

void ARedwoodCharacter::MarkMetadataDirty() {
  bMetadataDirty = true;
}

void ARedwoodCharacter::MarkEquippedInventoryDirty() {
  bEquippedInventoryDirty = true;
}

void ARedwoodCharacter::MarkNonequippedInventoryDirty() {
  bNonequippedInventoryDirty = true;
}

void ARedwoodCharacter::MarkDataDirty() {
  bDataDirty = true;
}

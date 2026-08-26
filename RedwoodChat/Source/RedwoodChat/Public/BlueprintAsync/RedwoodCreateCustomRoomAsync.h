// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "BlueprintAsync/RedwoodAsyncCommon.h"
#include "RedwoodChatClientSubsystem.h"

#include "RedwoodCreateCustomRoomAsync.generated.h"

UCLASS()
class REDWOODCHAT_API URedwoodCreateCustomRoomAsync
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()

public:
  virtual void Activate() override;

  /**
   * Create a custom room.
   *
   * Leave `Password` empty for a room anyone who knows the name can join, or
   * set one for a room that also needs the code. The code actually in force
   * comes back on the output pin, since the server generates one when you do
   * not supply it.
   *
   * `bCreateAsCharacter` chooses where the room lives: as your character, so it
   * belongs to this realm, or as your account, so it follows you between them.
   */
  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       DisplayName = "Create Custom Room",
       Category = "Redwood Chat",
       WorldContext = "WorldContextObject")
  )
  static URedwoodCreateCustomRoomAsync *CreateCustomRoom(
    URedwoodClientChatSubsystem *Target,
    UObject *WorldContextObject,
    FString Id,
    FString Password,
    bool bCreateAsCharacter
  );

  UPROPERTY(BlueprintAssignable)
  FRedwoodChatRoomCreatedDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientChatSubsystem *Target;

  FString Id;
  FString Password;
  bool bCreateAsCharacter = false;
};

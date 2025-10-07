// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "SIOJsonObject.h"

#include "RedwoodAsyncCommon.h"
#include "RedwoodClientGameSubsystem.h"

#include "RedwoodFriendsAsync.generated.h"

UCLASS()
class REDWOOD_API URedwoodAsync_SearchForPlayers
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()
public:
  virtual void Activate() override {
    Target->SearchForPlayers(
      UsernameOrNickname,
      bIncludePartialMatches,
      FRedwoodListFriendsOutputDelegate::CreateLambda([this](auto Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      })
    );
  };

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodAsync_SearchForPlayers *SearchForPlayers(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString UsernameOrNickname,
    bool bIncludePartialMatches
  ) {
    URedwoodAsync_SearchForPlayers *Action =
      NewObject<URedwoodAsync_SearchForPlayers>();
    Action->Target = Target;
    Action->UsernameOrNickname = UsernameOrNickname;
    Action->bIncludePartialMatches = bIncludePartialMatches;
    Action->RegisterWithGameInstance(WorldContextObject);
    return Action;
  };

  UPROPERTY(BlueprintAssignable)
  FRedwoodListFriendsOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  UPROPERTY()
  FString UsernameOrNickname;

  UPROPERTY()
  bool bIncludePartialMatches;
};

UCLASS()
class REDWOOD_API URedwoodAsync_ListFriends : public UBlueprintAsyncActionBase {
  GENERATED_BODY()
public:
  virtual void Activate() override {
    Target->ListFriends(
      Filter,
      FRedwoodListFriendsOutputDelegate::CreateLambda([this](auto Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      })
    );
  };

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodAsync_ListFriends *ListFriends(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    ERedwoodFriendListType Filter
  ) {
    URedwoodAsync_ListFriends *Action = NewObject<URedwoodAsync_ListFriends>();
    Action->Target = Target;
    Action->Filter = Filter;
    Action->RegisterWithGameInstance(WorldContextObject);
    return Action;
  };

  UPROPERTY(BlueprintAssignable)
  FRedwoodListFriendsOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  UPROPERTY()
  ERedwoodFriendListType Filter = ERedwoodFriendListType::Unknown;
};

UCLASS()
class REDWOOD_API URedwoodAsync_RequestFriend
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()
public:
  virtual void Activate() override {
    Target->RequestFriend(
      OtherPlayerId,
      FRedwoodErrorOutputDelegate::CreateLambda([this](auto Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      })
    );
  };

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodAsync_RequestFriend *RequestFriend(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString OtherPlayerId
  ) {
    URedwoodAsync_RequestFriend *Action =
      NewObject<URedwoodAsync_RequestFriend>();
    Action->Target = Target;
    Action->OtherPlayerId = OtherPlayerId;
    Action->RegisterWithGameInstance(WorldContextObject);
    return Action;
  };

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  UPROPERTY()
  FString OtherPlayerId;
};

UCLASS()
class REDWOOD_API URedwoodAsync_RemoveFriend
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()
public:
  virtual void Activate() override {
    Target->RemoveFriend(
      OtherPlayerId,
      FRedwoodErrorOutputDelegate::CreateLambda([this](auto Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      })
    );
  };

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodAsync_RemoveFriend *RemoveFriend(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString OtherPlayerId
  ) {
    URedwoodAsync_RemoveFriend *Action =
      NewObject<URedwoodAsync_RemoveFriend>();
    Action->Target = Target;
    Action->OtherPlayerId = OtherPlayerId;
    Action->RegisterWithGameInstance(WorldContextObject);
    return Action;
  };

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  UPROPERTY()
  FString OtherPlayerId;
};

UCLASS()
class REDWOOD_API URedwoodAsync_RespondToFriendRequest
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()
public:
  virtual void Activate() override {
    Target->RespondToFriendRequest(
      OtherPlayerId,
      bAccept,
      FRedwoodErrorOutputDelegate::CreateLambda([this](auto Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      })
    );
  };

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodAsync_RespondToFriendRequest *RespondToFriendRequest(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString OtherPlayerId,
    bool bAccept
  ) {
    URedwoodAsync_RespondToFriendRequest *Action =
      NewObject<URedwoodAsync_RespondToFriendRequest>();
    Action->Target = Target;
    Action->OtherPlayerId = OtherPlayerId;
    Action->bAccept = bAccept;
    Action->RegisterWithGameInstance(WorldContextObject);
    return Action;
  };

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  UPROPERTY()
  FString OtherPlayerId;

  UPROPERTY()
  bool bAccept;
};

UCLASS()
class REDWOOD_API URedwoodAsync_SetPlayerBlocked
  : public UBlueprintAsyncActionBase {
  GENERATED_BODY()
public:
  virtual void Activate() override {
    Target->SetPlayerBlocked(
      OtherPlayerId,
      bBlocked,
      FRedwoodErrorOutputDelegate::CreateLambda([this](auto Output) {
        OnOutput.Broadcast(Output);
        SetReadyToDestroy();
      })
    );
  };

  UFUNCTION(
    BlueprintCallable,
    meta =
      (BlueprintInternalUseOnly = "true",
       Category = "Redwood",
       WorldContext = "WorldContextObject")
  )
  static URedwoodAsync_SetPlayerBlocked *SetPlayerBlocked(
    URedwoodClientGameSubsystem *Target,
    UObject *WorldContextObject,
    FString OtherPlayerId,
    bool bBlocked
  ) {
    URedwoodAsync_SetPlayerBlocked *Action =
      NewObject<URedwoodAsync_SetPlayerBlocked>();
    Action->Target = Target;
    Action->OtherPlayerId = OtherPlayerId;
    Action->bBlocked = bBlocked;
    Action->RegisterWithGameInstance(WorldContextObject);
    return Action;
  };

  UPROPERTY(BlueprintAssignable)
  FRedwoodErrorOutputDynamicDelegate OnOutput;

  UPROPERTY()
  URedwoodClientGameSubsystem *Target;

  UPROPERTY()
  FString OtherPlayerId;

  UPROPERTY()
  bool bBlocked;
};
// Copyright Incanta LLC. All rights reserved.

#include "RedwoodPlayerState.h"

void ARedwoodPlayerState::SetClientReady_Implementation() {
  if (GetLocalRole() == ROLE_Authority) {
    bClientReady = true;
  }
}

bool ARedwoodPlayerState::SetClientReady_Validate() {
  return true;
}

void ARedwoodPlayerState::SetServerReady() {
  if (GetLocalRole() == ROLE_Authority) {
    bServerReady = true;
  }
}

void ARedwoodPlayerState::SetRedwoodCharacter(
  FRedwoodCharacterBackend InRedwoodCharacter
) {
  RedwoodCharacter = InRedwoodCharacter;

  FUniqueNetIdWrapper UniqueNetIdWrapper =
    UOnlineEngineInterface::Get()->CreateUniquePlayerIdWrapper(
      RedwoodCharacter.PlayerId + TEXT(":") + RedwoodCharacter.Id,
      FName(TEXT("RedwoodMMO"))
    );
  FUniqueNetIdRepl NetUniqueId(UniqueNetIdWrapper.GetUniqueNetId());

  SetUniqueId(NetUniqueId);

  SetPlayerName(RedwoodCharacter.Name);

  OnRedwoodCharacterUpdated.Broadcast();
}

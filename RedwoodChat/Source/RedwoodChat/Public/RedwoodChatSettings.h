// Copyright Incanta Games. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"

#include "RedwoodChatSettings.generated.h"

UCLASS(config = ProjectSettings, meta = (DisplayName = "Redwood"))
class REDWOODCHAT_API URedwoodChatSettings : public UDeveloperSettings {
  GENERATED_BODY()

public:
  URedwoodChatSettings() {
    CategoryName = TEXT("Plugins");
    SectionName = TEXT("Redwood");
  }

  /** The full URI to connect to the XMPP server */
  UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "General")
  FString XmppServerUri = "chat.localhost";

  static FString GetXmppServerUri() {
    URedwoodChatSettings *Settings = GetMutableDefault<URedwoodChatSettings>();
    FString Uri = Settings->XmppServerUri;

    // read from `redwood.json` if it exists
    FString JsonPath = FPaths::ProjectDir() / TEXT("redwood.json");
    if (FPaths::FileExists(JsonPath)) {
      FString Json;
      FFileHelper::LoadFileToString(Json, *JsonPath);
      TSharedPtr<FJsonObject> JsonObject;
      TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
      if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
        if (JsonObject->HasField(TEXT("xmppServerUri"))) {
          Uri = JsonObject->GetStringField(TEXT("xmppServerUri"));
        }
      }
    }

    return Uri;
  }

  virtual FName GetContainerName() const override {
    return "Project";
  }
};

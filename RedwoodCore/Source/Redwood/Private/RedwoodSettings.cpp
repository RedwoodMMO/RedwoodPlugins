// Copyright Incanta Games. All Rights Reserved.

#include "RedwoodSettings.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogRedwoodSettings, Log, All);

// OpenSSL's ossl_typ.h declares `typedef struct ui_st UI;`, which collides with
// Unreal's `namespace UI` from ObjectMacros.h (pulled in via the UObject header
// above). We don't use OpenSSL's UI type, so rename it across the OpenSSL include
// to avoid the redefinition error, then restore the token afterwards.
#define UI OpenSSL_UI
THIRD_PARTY_INCLUDES_START
#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#endif
#include <openssl/evp.h>
#if PLATFORM_WINDOWS
#include "Windows/HideWindowsPlatformTypes.h"
#endif
THIRD_PARTY_INCLUDES_END
#undef UI

// -----------------------------------------------------------------------
// Canonical JSON serialization
//
// Produces a deterministic byte sequence for a given FJsonValue tree so
// that a signature computed offline can be verified at runtime. Sorted
// object keys, no whitespace, escaped strings, lowercase keywords.
//
// Intentionally minimal — covers what `redwood.json` and similar
// signed-payload features need (strings, numbers, booleans, null,
// objects, arrays). Numbers are emitted via FString::SanitizeFloat
// stripping trailing zeros, which matches the canonical form most
// JCS-style libraries produce for the value ranges we sign. If you
// later sign payloads with scientific-notation or sub-millisecond
// floats, swap this for a real JCS implementation.
// -----------------------------------------------------------------------

namespace {

FString RwEscapeJsonString(const FString &In) {
  FString Out;
  Out.Reserve(In.Len() + 2);
  Out.AppendChar(TEXT('"'));
  for (TCHAR Ch : In) {
    switch (Ch) {
      case TEXT('\\'):
        Out += TEXT("\\\\");
        break;
      case TEXT('"'):
        Out += TEXT("\\\"");
        break;
      case TEXT('\b'):
        Out += TEXT("\\b");
        break;
      case TEXT('\f'):
        Out += TEXT("\\f");
        break;
      case TEXT('\n'):
        Out += TEXT("\\n");
        break;
      case TEXT('\r'):
        Out += TEXT("\\r");
        break;
      case TEXT('\t'):
        Out += TEXT("\\t");
        break;
      default:
        if (Ch < 0x20) {
          Out += FString::Printf(TEXT("\\u%04x"), (uint32)Ch);
        } else {
          Out.AppendChar(Ch);
        }
        break;
    }
  }
  Out.AppendChar(TEXT('"'));
  return Out;
}

FString SerializeNumber(double Value) {
  // Integer fast path keeps "0" / "60000" rendered without ".0"
  if (FMath::IsFinite(Value) && Value == FMath::TruncToDouble(Value) &&
      FMath::Abs(Value) < 1e15) {
    return FString::Printf(TEXT("%lld"), (int64)Value);
  }
  FString Str = FString::SanitizeFloat(Value);
  // SanitizeFloat returns e.g. "1.500000"; trim trailing zeros but keep
  // at least one digit after the decimal point.
  int32 DotIndex = INDEX_NONE;
  if (Str.FindChar(TEXT('.'), DotIndex)) {
    int32 EndIndex = Str.Len();
    while (EndIndex > DotIndex + 2 && Str[EndIndex - 1] == TEXT('0')) {
      --EndIndex;
    }
    Str = Str.Left(EndIndex);
  }
  return Str;
}

FString SerializeCanonicalJson(const TSharedPtr<FJsonValue> &Value);

FString SerializeCanonicalObject(const TSharedPtr<FJsonObject> &Object) {
  if (!Object.IsValid()) {
    return TEXT("{}");
  }
  TArray<FString> Keys;
  Object->Values.GetKeys(Keys);
  Keys.Sort();

  FString Out = TEXT("{");
  for (int32 i = 0; i < Keys.Num(); ++i) {
    if (i > 0) {
      Out.AppendChar(TEXT(','));
    }
    Out += RwEscapeJsonString(Keys[i]);
    Out.AppendChar(TEXT(':'));
    Out += SerializeCanonicalJson(Object->Values[Keys[i]]);
  }
  Out.AppendChar(TEXT('}'));
  return Out;
}

FString SerializeCanonicalJson(const TSharedPtr<FJsonValue> &Value) {
  if (!Value.IsValid() || Value->Type == EJson::Null) {
    return TEXT("null");
  }
  switch (Value->Type) {
    case EJson::Boolean:
      return Value->AsBool() ? TEXT("true") : TEXT("false");
    case EJson::Number:
      return SerializeNumber(Value->AsNumber());
    case EJson::String:
      return RwEscapeJsonString(Value->AsString());
    case EJson::Array: {
      const TArray<TSharedPtr<FJsonValue>> &Arr = Value->AsArray();
      FString Out = TEXT("[");
      for (int32 i = 0; i < Arr.Num(); ++i) {
        if (i > 0) {
          Out.AppendChar(TEXT(','));
        }
        Out += SerializeCanonicalJson(Arr[i]);
      }
      Out.AppendChar(TEXT(']'));
      return Out;
    }
    case EJson::Object:
      return SerializeCanonicalObject(Value->AsObject());
    default:
      return TEXT("null");
  }
}

// -----------------------------------------------------------------------
// Ed25519 signature verification via OpenSSL.
// -----------------------------------------------------------------------

bool VerifyEd25519(
  const TArray<uint8> &PublicKey,
  const TArray<uint8> &Signature,
  const TArray<uint8> &Message
) {
  if (PublicKey.Num() != 32) {
    UE_LOG(
      LogRedwoodSettings,
      Error,
      TEXT("Ed25519 public key must decode to 32 bytes (got %d)"),
      PublicKey.Num()
    );
    return false;
  }
  if (Signature.Num() != 64) {
    UE_LOG(
      LogRedwoodSettings,
      Error,
      TEXT("Ed25519 signature must decode to 64 bytes (got %d)"),
      Signature.Num()
    );
    return false;
  }

  EVP_PKEY *PKey = EVP_PKEY_new_raw_public_key(
    EVP_PKEY_ED25519, nullptr, PublicKey.GetData(), PublicKey.Num()
  );
  if (PKey == nullptr) {
    UE_LOG(
      LogRedwoodSettings,
      Error,
      TEXT("OpenSSL: EVP_PKEY_new_raw_public_key(ED25519) failed")
    );
    return false;
  }

  EVP_MD_CTX *Ctx = EVP_MD_CTX_new();
  if (Ctx == nullptr) {
    EVP_PKEY_free(PKey);
    UE_LOG(LogRedwoodSettings, Error, TEXT("OpenSSL: EVP_MD_CTX_new failed"));
    return false;
  }

  bool bOk = false;
  if (EVP_DigestVerifyInit(Ctx, nullptr, nullptr, nullptr, PKey) == 1) {
    const int Result = EVP_DigestVerify(
      Ctx,
      Signature.GetData(),
      Signature.Num(),
      Message.GetData(),
      Message.Num()
    );
    bOk = (Result == 1);
  } else {
    UE_LOG(
      LogRedwoodSettings, Error, TEXT("OpenSSL: EVP_DigestVerifyInit failed")
    );
  }

  EVP_MD_CTX_free(Ctx);
  EVP_PKEY_free(PKey);
  return bOk;
}

// -----------------------------------------------------------------------
// redwood.json loader.
//
// Returns null when no override should be applied (file missing, file
// unreadable, JSON malformed, or — critically — PublicSigningKey is set
// and the signature is missing or invalid).
// -----------------------------------------------------------------------

TSharedPtr<FJsonObject> LoadSignedRedwoodConfig(
  const FString &JsonPath, const FString &PublicSigningKeyBase64
) {
  if (!FPaths::FileExists(JsonPath)) {
    return nullptr;
  }

  FString Json;
  if (!FFileHelper::LoadFileToString(Json, *JsonPath)) {
    UE_LOG(
      LogRedwoodSettings,
      Warning,
      TEXT("Failed to read %s; ignoring override"),
      *JsonPath
    );
    return nullptr;
  }

  TSharedPtr<FJsonObject> JsonObject;
  TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
  if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid()) {
    UE_LOG(
      LogRedwoodSettings,
      Warning,
      TEXT("Failed to parse %s as JSON; ignoring override"),
      *JsonPath
    );
    return nullptr;
  }

  const bool bRequireSignature = !PublicSigningKeyBase64.IsEmpty();
  if (!bRequireSignature) {
    // Legacy / unsigned mode: trust the file as-is.
    return JsonObject;
  }

  // Signed mode: require a valid `signature` field over the canonical
  // form of the rest of the object.
  FString SignatureBase64;
  if (!JsonObject->TryGetStringField(TEXT("signature"), SignatureBase64) ||
      SignatureBase64.IsEmpty()) {
    UE_LOG(
      LogRedwoodSettings,
      Error,
      TEXT(
        "%s has no `signature` field but URedwoodSettings::PublicSigningKey "
        "is set; refusing to apply override"
      ),
      *JsonPath
    );
    return nullptr;
  }

  // Build the message to verify: same object with `signature` removed,
  // serialized canonically (sorted keys, no whitespace, deterministic
  // string/number formatting).
  TSharedPtr<FJsonObject> Unsigned = MakeShared<FJsonObject>();
  for (const auto &Pair : JsonObject->Values) {
    if (Pair.Key.Equals(TEXT("signature"), ESearchCase::CaseSensitive)) {
      continue;
    }
    Unsigned->SetField(Pair.Key, Pair.Value);
  }
  const FString CanonicalJson = SerializeCanonicalObject(Unsigned);

  TArray<uint8> PublicKeyBytes;
  if (!FBase64::Decode(PublicSigningKeyBase64, PublicKeyBytes)) {
    UE_LOG(
      LogRedwoodSettings,
      Error,
      TEXT(
        "URedwoodSettings::PublicSigningKey is not valid base64; refusing to "
        "apply override from %s"
      ),
      *JsonPath
    );
    return nullptr;
  }

  TArray<uint8> SignatureBytes;
  if (!FBase64::Decode(SignatureBase64, SignatureBytes)) {
    UE_LOG(
      LogRedwoodSettings,
      Error,
      TEXT(
        "%s `signature` field is not valid base64; refusing to apply override"
      ),
      *JsonPath
    );
    return nullptr;
  }

  // UTF-8 encode the canonical JSON for verification.
  FTCHARToUTF8 Utf8Converter(*CanonicalJson);
  TArray<uint8> MessageBytes;
  MessageBytes.Append(
    reinterpret_cast<const uint8 *>(Utf8Converter.Get()), Utf8Converter.Length()
  );

  if (!VerifyEd25519(PublicKeyBytes, SignatureBytes, MessageBytes)) {
    UE_LOG(
      LogRedwoodSettings,
      Error,
      TEXT(
        "%s `signature` failed Ed25519 verification against PublicSigningKey; "
        "refusing to apply override"
      ),
      *JsonPath
    );
    return nullptr;
  }

  return JsonObject;
}

} // namespace

FString URedwoodSettings::GetDirectorUri() {
  URedwoodSettings *Settings = GetMutableDefault<URedwoodSettings>();
  FString Uri = Settings->DirectorUri;

  // Operator kill switch: don't even look for the file if the override
  // is globally disabled. Skips the file existence check, signature
  // verification, and any logging — also the lowest-overhead path for
  // shipped builds that don't want to honor any local config override.
  if (!Settings->bRedwoodJsonEnabled) {
    return Uri;
  }

  const FString JsonPath = FPaths::ProjectDir() / TEXT("redwood.json");
  TSharedPtr<FJsonObject> JsonObject =
    LoadSignedRedwoodConfig(JsonPath, Settings->PublicSigningKey);
  if (JsonObject.IsValid() && JsonObject->HasField(TEXT("directorUri"))) {
    Uri = JsonObject->GetStringField(TEXT("directorUri"));
  }

  return Uri;
}

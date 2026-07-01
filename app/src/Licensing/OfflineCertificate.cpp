/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "Licensing/OfflineCertificate.h"

#include <openssl/evp.h>
#include <QCborMap>
#include <QCborValue>
#include <QCoreApplication>

//--------------------------------------------------------------------------------------------------
// Certificate format constants (must match the signing portal's output)
//--------------------------------------------------------------------------------------------------

static constexpr int kFormatVersion  = 1;
static constexpr int kSignatureBytes = 64;

//--------------------------------------------------------------------------------------------------
// Tier mapping
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps a certificate variant string to a feature tier (empty maps to None).
 */
Licensing::FeatureTier Licensing::tierFromCertVariant(const QString& variant)
{
  const auto lower = variant.toLower();

  if (lower.startsWith("enterprise"))
    return FeatureTier::Enterprise;

  if (!lower.isEmpty())
    return FeatureTier::Pro;

  return FeatureTier::None;
}

//--------------------------------------------------------------------------------------------------
// Parsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Splits a base64-framed certificate into its CBOR payload and 64-byte signature.
 */
bool Licensing::parseCertificate(const QByteArray& framed,
                                 QByteArray& payload,
                                 QByteArray& signature)
{
  const auto decoded =
    QByteArray::fromBase64(framed.trimmed(), QByteArray::AbortOnBase64DecodingErrors);
  if (decoded.size() <= kSignatureBytes)
    return false;

  payload   = decoded.left(decoded.size() - kSignatureBytes);
  signature = decoded.right(kSignatureBytes);
  return true;
}

//--------------------------------------------------------------------------------------------------
// Signature verification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when the Ed25519 signature over payload validates against publicKey.
 */
[[nodiscard]] static bool verifySignature(const QByteArray& payload,
                                          const QByteArray& signature,
                                          const std::array<std::uint8_t, 32>& publicKey)
{
  EVP_PKEY* key =
    EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, nullptr, publicKey.data(), publicKey.size());
  if (!key)
    return false;

  EVP_MD_CTX* ctx = EVP_MD_CTX_new();
  bool ok         = false;
  if (ctx && EVP_DigestVerifyInit(ctx, nullptr, nullptr, nullptr, key) == 1) {
    const auto* sig = reinterpret_cast<const unsigned char*>(signature.constData());
    const auto* msg = reinterpret_cast<const unsigned char*>(payload.constData());
    ok =
      EVP_DigestVerify(
        ctx, sig, static_cast<size_t>(signature.size()), msg, static_cast<size_t>(payload.size()))
      == 1;
  }

  EVP_MD_CTX_free(ctx);
  EVP_PKEY_free(key);
  return ok;
}

//--------------------------------------------------------------------------------------------------
// Field decoding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Decodes the CBOR payload into fields; false on any structural mismatch.
 */
[[nodiscard]] static bool decodeFields(const QByteArray& payload,
                                       Licensing::CertificateFields& fields)
{
  const auto value = QCborValue::fromCbor(payload);
  if (!value.isMap())
    return false;

  const auto map     = value.toMap();
  const auto version = map[QStringLiteral("v")];
  const auto machine = map[QStringLiteral("mid")];
  const auto variant = map[QStringLiteral("var")];
  const auto inst    = map[QStringLiteral("iid")];
  const auto issued  = map[QStringLiteral("iat")];
  const auto expires = map[QStringLiteral("exp")];

  if (!version.isInteger() || !machine.isString() || !variant.isString() || !issued.isInteger()
      || !expires.isInteger())
    return false;

  fields.version    = static_cast<int>(version.toInteger());
  fields.machineId  = machine.toString();
  fields.variant    = variant.toString();
  fields.instanceId = inst.isString() ? inst.toString() : QString();
  fields.issuedAt   = issued.toInteger();
  fields.expiresAt  = expires.toInteger();
  return true;
}

//--------------------------------------------------------------------------------------------------
// Verification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Verifies signature first (security invariant), then machine binding and expiry.
 */
Licensing::CertStatus Licensing::verifyCertificate(const QByteArray& framed,
                                                   const std::array<std::uint8_t, 32>& publicKey,
                                                   const QString& machineId,
                                                   qint64 nowSecs,
                                                   CertificateFields& fields)
{
  QByteArray payload;
  QByteArray signature;
  if (!parseCertificate(framed, payload, signature))
    return CertStatus::Malformed;

  if (!verifySignature(payload, signature, publicKey))
    return CertStatus::BadSignature;

  if (!decodeFields(payload, fields) || fields.version != kFormatVersion)
    return CertStatus::Malformed;

  if (fields.machineId != machineId)
    return CertStatus::MachineMismatch;

  if (fields.expiresAt <= nowSecs)
    return CertStatus::Expired;

  if (tierFromCertVariant(fields.variant) == FeatureTier::None)
    return CertStatus::WrongTier;

  return CertStatus::Valid;
}

//--------------------------------------------------------------------------------------------------
// Diagnostics
//--------------------------------------------------------------------------------------------------

/**
 * @brief Human-readable reason for a verification status (for UI / API errors).
 */
QString Licensing::certStatusReason(CertStatus status)
{
  switch (status) {
    case CertStatus::Valid:
      return QCoreApplication::translate("OfflineLicense", "License certificate is valid.");
    case CertStatus::BadSignature:
      return QCoreApplication::translate("OfflineLicense",
                                         "The certificate signature is invalid or corrupted.");
    case CertStatus::MachineMismatch:
      return QCoreApplication::translate("OfflineLicense",
                                         "This certificate was issued for a different device.");
    case CertStatus::Expired:
      return QCoreApplication::translate("OfflineLicense", "This certificate has expired.");
    case CertStatus::WrongTier:
      return QCoreApplication::translate("OfflineLicense",
                                         "This certificate does not grant a valid license tier.");
    case CertStatus::Malformed:
    default:
      return QCoreApplication::translate("OfflineLicense",
                                         "The certificate file is malformed or unreadable.");
  }
}

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

#pragma once

#include <array>
#include <cstdint>
#include <QByteArray>
#include <QString>

#include "Licensing/CommercialToken.h"

namespace Licensing {

/**
 * @brief Outcome of verifying an offline license certificate.
 */
enum class CertStatus : quint8 {
  Valid           = 0,
  Malformed       = 1,
  BadSignature    = 2,
  MachineMismatch = 3,
  Expired         = 4,
  WrongTier       = 5,
};

/**
 * @brief Decoded certificate payload fields (valid only once CertStatus::Valid).
 */
struct CertificateFields {
  int version = 0;
  QString machineId;
  QString variant;
  QString instanceId;
  qint64 issuedAt  = 0;
  qint64 expiresAt = 0;
};

/**
 * @brief Maps a certificate variant string to a feature tier (empty maps to None).
 */
[[nodiscard]] FeatureTier tierFromCertVariant(const QString& variant);

/**
 * @brief Splits a base64-framed certificate into its CBOR payload and 64-byte signature.
 */
[[nodiscard]] bool parseCertificate(const QByteArray& framed,
                                    QByteArray& payload,
                                    QByteArray& signature);

/**
 * @brief Verifies signature, machine binding, and expiry; fills fields on success.
 */
[[nodiscard]] CertStatus verifyCertificate(const QByteArray& framed,
                                           const std::array<std::uint8_t, 32>& publicKey,
                                           const QString& machineId,
                                           qint64 nowSecs,
                                           CertificateFields& fields);

/**
 * @brief Human-readable reason for a verification status (for UI / API errors).
 */
[[nodiscard]] QString certStatusReason(CertStatus status);

}  // namespace Licensing

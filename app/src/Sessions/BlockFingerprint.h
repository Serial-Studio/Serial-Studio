/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QByteArray>
#include <QCryptographicHash>
#include <QString>

namespace Sessions {

/**
 * @brief Canonical row serialization behind the spec-0044 capture fingerprints. Kept in its own
 *        pair, free of the exporter's includes, because Sessions::Verifier re-derives nothing: it
 *        re-hashes an archive through these exact functions, so the layout has one definition.
 */

/**
 * @brief Feeds one raw_bytes row into a fingerprint hash using the spec-0044 canonical layout.
 */
void hashRawChunk(QCryptographicHash& hash, qint64 ns, int deviceId, const QByteArray& data);

/**
 * @brief Feeds one readings row into a fingerprint hash using the spec-0044 canonical layout.
 */
void hashReadingRow(QCryptographicHash& hash,
                    qint64 ns,
                    qint64 uniqueId,
                    double rawNumeric,
                    const QString& rawString,
                    double finalNumeric,
                    const QString& finalString,
                    bool isNumeric);

/**
 * @brief Feeds one `blocks` row into a fingerprint hash (spec 0055). Both blobs are already
 *        canonical little-endian, so the digest is machine-independent.
 */
void hashBlockRow(QCryptographicHash& hash,
                  qint64 uniqueId,
                  qint64 t0Ns,
                  qint64 dtNs,
                  qint64 frames,
                  const QByteArray& values,
                  const QByteArray& rawValues,
                  const QByteArray& texts);

}  // namespace Sessions

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

#include "Sessions/BlockFingerprint.h"

#include <bit>
#include <cmath>
#include <QtEndian>

//--------------------------------------------------------------------------------------------------
// Fingerprint canonical serialization (spec 0044, shared with Sessions::Verifier)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends a little-endian 64-bit integer to the hash.
 */
static void hashLe64(QCryptographicHash& hash, quint64 value)
{
  char buffer[sizeof(quint64)];
  qToLittleEndian(value, buffer);
  hash.addData(QByteArrayView(buffer, sizeof(buffer)));
}

/**
 * @brief Appends an IEEE-754 double as its little-endian bit pattern to the hash. NaN folds to
 *        0.0 because SQLite stores NaN as NULL and the verifier reads NULL back as 0.0: the
 *        digest must cover the value the archive actually round-trips.
 */
static void hashDouble(QCryptographicHash& hash, double value)
{
  const double canonical = std::isnan(value) ? 0.0 : value;
  hashLe64(hash, std::bit_cast<quint64>(canonical));
}

/**
 * @brief Appends a length-prefixed UTF-8 string to the hash.
 */
static void hashString(QCryptographicHash& hash, const QString& value)
{
  const QByteArray utf8 = value.toUtf8();
  hashLe64(hash, static_cast<quint64>(utf8.size()));
  hash.addData(utf8);
}

/**
 * @brief Feeds one raw_bytes row into a fingerprint hash using the spec-0044 canonical layout.
 */
void Sessions::hashRawChunk(QCryptographicHash& hash,
                            qint64 ns,
                            int deviceId,
                            const QByteArray& data)
{
  hashLe64(hash, static_cast<quint64>(ns));
  hashLe64(hash, static_cast<quint64>(deviceId));
  hashLe64(hash, static_cast<quint64>(data.size()));
  hash.addData(data);
}

/**
 * @brief Feeds one readings row into a fingerprint hash using the spec-0044 canonical layout.
 */
void Sessions::hashReadingRow(QCryptographicHash& hash,
                              qint64 ns,
                              qint64 uniqueId,
                              double rawNumeric,
                              const QString& rawString,
                              double finalNumeric,
                              const QString& finalString,
                              bool isNumeric)
{
  hashLe64(hash, static_cast<quint64>(ns));
  hashLe64(hash, static_cast<quint64>(uniqueId));
  hashDouble(hash, rawNumeric);
  hashString(hash, rawString);
  hashDouble(hash, finalNumeric);
  hashString(hash, finalString);
  hashLe64(hash, isNumeric ? 1 : 0);
}

/**
 * @brief Feeds one `blocks` row into a fingerprint hash (spec 0055). Both blobs are canonical
 *        little-endian already, so the digest is machine-independent.
 */
void Sessions::hashBlockRow(QCryptographicHash& hash,
                            qint64 uniqueId,
                            qint64 t0Ns,
                            qint64 dtNs,
                            qint64 frames,
                            const QByteArray& values,
                            const QByteArray& rawValues,
                            const QByteArray& texts)
{
  hashLe64(hash, static_cast<quint64>(uniqueId));
  hashLe64(hash, static_cast<quint64>(t0Ns));
  hashLe64(hash, static_cast<quint64>(dtNs));
  hashLe64(hash, static_cast<quint64>(frames));
  hashLe64(hash, static_cast<quint64>(values.size()));
  hash.addData(values);
  hashLe64(hash, static_cast<quint64>(rawValues.size()));
  hash.addData(rawValues);
  hashLe64(hash, static_cast<quint64>(texts.size()));
  hash.addData(texts);
}

/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <cmath>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <vector>

#include "Core/SSAssert.h"
#include "SerialStudio.h"

/**
 * @file MirrorProtocol.h
 * @brief Wire version 1 of the remote-dashboard mirror (spec 0040).
 *
 * The normative contract is doc/claude/specs/0040-remote-dashboard/wire-protocol.md; the
 * reference decoder is tests/utils/mirror_client.py and the conformance suite is
 * tests/unit/test_mirror_protocol.py. This header is pure encode/decode: no singletons, no
 * UI, no sockets, so both the publisher and a future in-process viewer share one codec.
 *
 * The format is positional on purpose: the structure message carries the ordered
 * (sourceId, uniqueId) identity list once per epoch, every snapshot after it is a bare array
 * of numbers, and the layout hash is what makes that safe. A client that cannot reproduce the
 * announced hash over the received identity list refuses the structure rather than rendering
 * values against a layout it did not verify.
 */

namespace API::Mirror {

//--------------------------------------------------------------------------------------------------
// Protocol constants
//--------------------------------------------------------------------------------------------------

inline constexpr int kWireVersion = 1;

inline constexpr int kHzMin     = 1;
inline constexpr int kHzMax     = 60;
inline constexpr int kHzDefault = 20;

inline constexpr int kPrecisionMin = 0;
inline constexpr int kPrecisionMax = 17;

inline constexpr int kHeartbeatIntervalMs = 1000;
inline constexpr int kStructureChunkBytes = 512 * 1024;
inline constexpr int kMaxStructureParts   = 64;

inline constexpr quint64 kFnv64Offset = 0xcbf29ce484222325ULL;
inline constexpr quint64 kFnv64Prime  = 0x100000001b3ULL;

/**
 * @brief Top-level key of every server push; invisible to clients that key-sniff the existing
 *        frames / data / event pushes, none of which carry it.
 */
inline constexpr const char* kPushKey = "mirror";

namespace Kind {
inline constexpr const char* Structure      = "structure";
inline constexpr const char* StructureChunk = "structureChunk";
inline constexpr const char* Snapshot       = "snapshot";
inline constexpr const char* Heartbeat      = "heartbeat";
}  // namespace Kind

namespace Command {
inline constexpr const char* GetInfo      = "mirror.getInfo";
inline constexpr const char* GetStructure = "mirror.getStructure";
inline constexpr const char* Subscribe    = "mirror.subscribe";
inline constexpr const char* SetRate      = "mirror.setRate";
inline constexpr const char* Unsubscribe  = "mirror.unsubscribe";
}  // namespace Command

namespace ErrorCode {
inline constexpr const char* VersionMismatch   = "MIRROR_VERSION_MISMATCH";
inline constexpr const char* RateOutOfRange    = "MIRROR_RATE_OUT_OF_RANGE";
inline constexpr const char* NotSubscribed     = "MIRROR_NOT_SUBSCRIBED";
inline constexpr const char* ViewerLimit       = "MIRROR_VIEWER_LIMIT";
inline constexpr const char* StructureTooLarge = "MIRROR_STRUCTURE_TOO_LARGE";
}  // namespace ErrorCode

namespace NonFinite {
inline constexpr const char* NaN         = "nan";
inline constexpr const char* Infinity    = "inf";
inline constexpr const char* NegInfinity = "-inf";
}  // namespace NonFinite

//--------------------------------------------------------------------------------------------------
// Value types
//--------------------------------------------------------------------------------------------------

/**
 * @brief Positional identity of one dataset. uniqueId is the dataset's persisted identity, or
 *        the legacy dataset_unique_id() value when it was never assigned.
 */
struct DatasetId {
  int sourceId = 0;
  int uniqueId = 0;
};

/**
 * @brief One positional snapshot slot. Strings and non-finite doubles cannot ride the numeric
 *        array (JSON has no NaN and a per-tick string costs what the positional format exists
 *        to avoid), so they leave a null behind and travel in the sparse side maps.
 */
struct SnapshotValue {
  enum class Kind : quint8 {
    Empty,
    Number,
    Text,
    NonFinite
  };

  Kind kind     = Kind::Empty;
  double number = 0;
  QString text;
};

//--------------------------------------------------------------------------------------------------
// Layout hash
//--------------------------------------------------------------------------------------------------

/**
 * @brief FNV-1a 64 over the seed bytes "mirror-v1\n" followed by "<sourceId>:<uniqueId>;" per
 *        entry, in list order, as 16 lowercase hex digits. Any reorder, insertion, or removal
 *        changes it, which is the whole safety argument for a positional value format.
 */
[[nodiscard]] inline QString layoutHash(const std::vector<DatasetId>& datasets)
{
  quint64 hash      = kFnv64Offset;
  const auto absorb = [&hash](const QByteArray& bytes) {
    for (const char byte : bytes) {
      hash ^= static_cast<quint8>(byte);
      hash *= kFnv64Prime;
    }
  };

  absorb(QByteArrayLiteral("mirror-v1\n"));
  for (const auto& entry : datasets)
    absorb(QByteArray::number(entry.sourceId) + ':' + QByteArray::number(entry.uniqueId) + ';');

  return QString::number(hash, 16).rightJustified(16, QLatin1Char('0'));
}

//--------------------------------------------------------------------------------------------------
// Encoding helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rounds to @p digits significant digits; full round-trip precision when digits is out
 *        of the [1, 16] band, which is what a plot needs and the wire default. Routed through
 *        the 'g' formatter rather than a pow() scale so the result is the decimal a client
 *        asked for instead of the nearest binary double to it.
 */
[[nodiscard]] inline double roundSignificant(const double value, const int digits)
{
  if (digits < 1 || digits > 16 || value == 0.0 || !std::isfinite(value))
    return value;

  return SerialStudio::toDouble(QString::number(value, 'g', digits));
}

/**
 * @brief Wraps one payload object in the push envelope and serializes it as an NDJSON line,
 *        the framing every other message on this socket already uses.
 */
[[nodiscard]] inline QByteArray encodeLine(const QJsonObject& payload)
{
  SS_ASSERT(!payload.isEmpty(), return QByteArray());
  SS_ASSERT_LOG(payload.contains(QLatin1String("kind")));

  QJsonObject push;
  push.insert(QLatin1String(kPushKey), payload);
  return QJsonDocument(push).toJson(QJsonDocument::Compact) + '\n';
}

/**
 * @brief Builds a `structure` payload: the ordered identity list, the layout hash over it, the
 *        remote's display-facing state, and the serialized project document verbatim.
 */
[[nodiscard]] inline QJsonObject encodeStructure(const quint64 epoch,
                                                 const std::vector<DatasetId>& datasets,
                                                 const std::vector<int>& sourceIds,
                                                 const QJsonObject& project,
                                                 const int operationMode,
                                                 const double plotTimeRange,
                                                 const bool frozen,
                                                 const qint64 originUnixMs)
{
  SS_ASSERT_LOG(epoch > 0);
  SS_ASSERT_LOG(plotTimeRange > 0);

  QJsonArray ids;
  for (const auto& entry : datasets) {
    QJsonArray pair;
    pair.append(entry.sourceId);
    pair.append(entry.uniqueId);
    ids.append(pair);
  }

  QJsonArray sources;
  for (const int sourceId : sourceIds)
    sources.append(sourceId);

  QJsonObject clock;
  clock.insert(QLatin1String("domain"), QLatin1String("monotonic-relative"));
  clock.insert(QLatin1String("originUnixMs"), originUnixMs);

  QJsonObject payload;
  payload.insert(QLatin1String("kind"), QLatin1String(Kind::Structure));
  payload.insert(QLatin1String("wireVersion"), kWireVersion);
  payload.insert(QLatin1String("epoch"), static_cast<qint64>(epoch));
  payload.insert(QLatin1String("layoutHash"), layoutHash(datasets));
  payload.insert(QLatin1String("sourceIds"), sources);
  payload.insert(QLatin1String("datasets"), ids);
  payload.insert(QLatin1String("operationMode"), operationMode);
  payload.insert(QLatin1String("plotTimeRange"), plotTimeRange);
  payload.insert(QLatin1String("frozen"), frozen);
  payload.insert(QLatin1String("clock"), clock);
  payload.insert(QLatin1String("project"), project);
  return payload;
}

/**
 * @brief Builds a `snapshot` payload. The positional array always carries one slot per declared
 *        dataset, so a client can compare its length against the held structure and drop the
 *        message instead of applying values against a layout they do not match.
 */
[[nodiscard]] inline QJsonObject encodeSnapshot(const quint64 epoch,
                                                const quint64 seq,
                                                const std::vector<qint64>& tNs,
                                                const std::vector<SnapshotValue>& values,
                                                const int precision)
{
  SS_ASSERT_LOG(epoch > 0);
  SS_ASSERT_LOG(seq > 0);

  QJsonArray times;
  for (const qint64 stamp : tNs)
    times.append(stamp);

  QJsonArray valueArray;
  QJsonObject strings;
  QJsonObject nonFinite;
  for (std::size_t i = 0; i < values.size(); ++i) {
    const auto& value = values[i];
    const auto key    = QString::number(i);

    if (value.kind == SnapshotValue::Kind::Number)
      valueArray.append(roundSignificant(value.number, precision));
    else
      valueArray.append(QJsonValue::Null);

    if (value.kind == SnapshotValue::Kind::Text)
      strings.insert(key, value.text);
    else if (value.kind == SnapshotValue::Kind::NonFinite)
      nonFinite.insert(key, value.text);
  }

  QJsonObject payload;
  payload.insert(QLatin1String("kind"), QLatin1String(Kind::Snapshot));
  payload.insert(QLatin1String("epoch"), static_cast<qint64>(epoch));
  payload.insert(QLatin1String("seq"), static_cast<qint64>(seq));
  payload.insert(QLatin1String("tNs"), times);
  payload.insert(QLatin1String("values"), valueArray);

  if (!strings.isEmpty())
    payload.insert(QLatin1String("strings"), strings);

  if (!nonFinite.isEmpty())
    payload.insert(QLatin1String("nonFinite"), nonFinite);

  return payload;
}

/**
 * @brief Builds a `heartbeat` payload, repeating the last emitted sequence so an idle-but-healthy
 *        capture reads as "connected, no data" instead of as a dead link.
 */
[[nodiscard]] inline QJsonObject encodeHeartbeat(const quint64 epoch, const quint64 seq)
{
  SS_ASSERT_LOG(epoch > 0);

  QJsonObject payload;
  payload.insert(QLatin1String("kind"), QLatin1String(Kind::Heartbeat));
  payload.insert(QLatin1String("epoch"), static_cast<qint64>(epoch));
  payload.insert(QLatin1String("seq"), static_cast<qint64>(seq));
  return payload;
}

/**
 * @brief Splits an oversized structure into `structureChunk` payloads. The object is base64'd
 *        before slicing so a boundary can never land inside a multi-byte UTF-8 sequence; the
 *        33% inflation is paid once per epoch on a human-paced event. Returns an empty vector
 *        when the structure needs more parts than the cap allows.
 */
[[nodiscard]] inline std::vector<QJsonObject> chunkStructure(
  const QJsonObject& structure, const int chunkBytes = kStructureChunkBytes)
{
  SS_ASSERT(chunkBytes > 0, return {});
  SS_ASSERT(!structure.isEmpty(), return {});

  const auto blob = QJsonDocument(structure).toJson(QJsonDocument::Compact).toBase64();
  const int parts = static_cast<int>((blob.size() + chunkBytes - 1) / chunkBytes);
  if (parts > kMaxStructureParts)
    return {};

  const qint64 epoch = structure.value(QLatin1String("epoch")).toInteger(0);
  std::vector<QJsonObject> chunks;
  chunks.reserve(static_cast<std::size_t>(qMax(1, parts)));

  for (int part = 0; part < qMax(1, parts); ++part) {
    QJsonObject payload;
    payload.insert(QLatin1String("kind"), QLatin1String(Kind::StructureChunk));
    payload.insert(QLatin1String("epoch"), epoch);
    payload.insert(QLatin1String("part"), part);
    payload.insert(QLatin1String("parts"), qMax(1, parts));
    payload.insert(QLatin1String("data"),
                   QString::fromLatin1(blob.mid(part * chunkBytes, chunkBytes)));
    chunks.push_back(payload);
  }

  return chunks;
}

}  // namespace API::Mirror

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

#include <optional>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <vector>

#include "SerialStudio.h"
#include "Sessions/StreamBlockCodec.h"

namespace Sessions {

/**
 * @brief One materialised per-sample reading, the shape every session reader worked in before
 *        spec 0055 unified the storage into blocks.
 */
struct ReadingRow {
  qint64 timestampNs  = 0;
  int uniqueId        = 0;
  double rawNumeric   = 0.0;
  double finalNumeric = 0.0;
  QString rawString;
  QString finalString;
  bool isNumeric = true;
};

/**
 * @brief Column list every `blocks` SELECT feeding decodeBlockRow() must use, in this order.
 *        Sharing the list is what keeps the query and the decoder from drifting apart, the same
 *        reason the sample codec's two halves live in one header.
 */
// Ceiling on a block's archive-declared sample count, bounding every resize made from it
inline constexpr qint64 kMaxBlockFrames = 1 << 22;

inline constexpr auto kBlockColumns = "unique_id, t0_ns, dt_ns, frames, is_numeric, "
                                      "values_blob, raw_values, texts, raw_texts, times";

/**
 * @brief Expands a block's timing into one absolute nanosecond stamp per sample: a uniform grid
 *        derives them from t0 + i * dt, an irregular block (dt 0) reads its explicit offsets.
 */
[[nodiscard]] inline bool expandBlockTimes(
  qint64 t0Ns, qint64 dtNs, qint64 frames, const QByteArray& timesBlob, std::vector<qint64>& out)
{
  if (frames < 0 || frames > kMaxBlockFrames)
    return false;

  if (dtNs != 0) {
    out.resize(static_cast<std::size_t>(frames));
    for (qint64 i = 0; i < frames; ++i)
      out[static_cast<std::size_t>(i)] = t0Ns + i * dtNs;

    return true;
  }

  std::vector<qint64> offsets;
  if (!unpackStreamTimes(timesBlob, frames, offsets))
    return false;

  out.resize(static_cast<std::size_t>(frames));
  for (qint64 i = 0; i < frames; ++i)
    out[static_cast<std::size_t>(i)] = t0Ns + offsets[static_cast<std::size_t>(i)];

  return true;
}

/**
 * @brief Decodes the `blocks` row @p query currently points at into @p out, appending one
 *        ReadingRow per sample. Returns false on a malformed row rather than emitting partial
 *        samples: a blob whose length disagrees with `frames` means the archive is damaged, and
 *        half-decoding it would silently shift every later value onto the wrong timestamp.
 */
[[nodiscard]] inline bool decodeBlockRow(const QSqlQuery& query, std::vector<ReadingRow>& out)
{
  const int uniqueId       = query.value(0).toInt();
  const qint64 t0Ns        = query.value(1).toLongLong();
  const qint64 dtNs        = query.value(2).toLongLong();
  const qint64 frames      = query.value(3).toLongLong();
  const bool isNumeric     = query.value(4).toInt() != 0;
  const QByteArray values  = query.value(5).toByteArray();
  const QByteArray raws    = query.value(6).toByteArray();
  const QByteArray texts   = query.value(7).toByteArray();
  const QByteArray rawText = query.value(8).toByteArray();
  const QByteArray times   = query.value(9).toByteArray();

  if (frames <= 0)
    return true;

  if (frames > kMaxBlockFrames)
    return false;

  std::vector<double> finals;
  if (!unpackStreamSamples(values, frames, finals))
    return false;

  std::vector<qint64> stamps;
  if (!expandBlockTimes(t0Ns, dtNs, frames, times, stamps))
    return false;

  std::vector<double> rawValues;
  const bool hasRaw = !raws.isEmpty();
  if (hasRaw && !unpackStreamSamples(raws, frames, rawValues))
    return false;

  std::vector<QString> finalStrings;
  const bool hasText = !texts.isEmpty();
  if (hasText && !unpackStreamText(texts, frames, finalStrings))
    return false;

  std::vector<QString> rawStrings;
  const bool hasRawText = !rawText.isEmpty();
  if (hasRawText && !unpackStreamText(rawText, frames, rawStrings))
    return false;

  out.reserve(out.size() + static_cast<std::size_t>(frames));
  for (qint64 i = 0; i < frames; ++i) {
    const auto slot = static_cast<std::size_t>(i);

    ReadingRow row;
    row.timestampNs  = stamps[slot];
    row.uniqueId     = uniqueId;
    row.finalNumeric = finals[slot];
    row.rawNumeric   = hasRaw ? rawValues[slot] : finals[slot];
    row.isNumeric    = isNumeric;

    if (hasText)
      row.finalString = finalStrings[slot];

    row.rawString = hasRawText ? rawStrings[slot] : row.finalString;
    out.push_back(std::move(row));
  }

  return true;
}

/**
 * @brief Whether @p sessionId's samples live in `blocks` (spec 0055) rather than the legacy
 *        `readings` table. Probed per session, not from PRAGMA user_version: opening a v1 archive
 *        with a current build migrates its schema, so the version says nothing about where an
 *        already-recorded session's data actually sits.
 */
[[nodiscard]] inline bool sessionUsesBlocks(const QSqlDatabase& db, int sessionId)
{
  QSqlQuery probe(db);
  if (!probe.exec(QStringLiteral("SELECT name FROM sqlite_master WHERE type='table' AND "
                                 "name='blocks'")))
    return false;

  if (!probe.next())
    return false;

  QSqlQuery rows(db);
  rows.prepare(QStringLiteral("SELECT 1 FROM blocks WHERE session_id = ? LIMIT 1"));
  rows.addBindValue(sessionId);
  if (!rows.exec())
    return false;

  return rows.next();
}

/**
 * @brief Streams a session's readings in time order from whichever storage holds them, decoding a
 *        spec-0055 archive one block at a time so memory stays bounded by a block rather than by
 *        session length. Lets a consumer comparing two archives keep one path when the two sides
 *        sit in different storages (spec 0044 / 0047).
 */
class ReadingCursor {
public:
  /**
   * @brief Opens the cursor over @p sessionId, optionally restricted to one dataset. Returns false
   *        when the underlying query fails, which callers must report as an error rather than as
   *        an empty comparison.
   */
  [[nodiscard]] bool open(const QSqlDatabase& db, int sessionId, qint64 uniqueId = -1)
  {
    m_blocks  = sessionUsesBlocks(db, sessionId);
    m_damaged = false;
    m_buffer.clear();
    m_cursor = 0;

    m_query.emplace(db);
    m_query->setForwardOnly(true);

    if (m_blocks) {
      const auto sql = uniqueId >= 0
                       ? QStringLiteral("SELECT %1 FROM blocks WHERE session_id = ? AND "
                                        "unique_id = ? ORDER BY t0_ns, block_id")
                       : QStringLiteral("SELECT %1 FROM blocks WHERE session_id = ? "
                                        "ORDER BY t0_ns, block_id");
      m_query->prepare(sql.arg(QLatin1String(kBlockColumns)));
    } else {
      const auto sql =
        uniqueId >= 0
          ? QStringLiteral("SELECT timestamp_ns, unique_id, raw_numeric_value, raw_string_value, "
                           "final_numeric_value, final_string_value, is_numeric FROM readings "
                           "WHERE session_id = ? AND unique_id = ? ORDER BY timestamp_ns, "
                           "reading_id")
          : QStringLiteral("SELECT timestamp_ns, unique_id, raw_numeric_value, raw_string_value, "
                           "final_numeric_value, final_string_value, is_numeric FROM readings "
                           "WHERE session_id = ? ORDER BY timestamp_ns, reading_id");
      m_query->prepare(sql);
    }

    m_query->addBindValue(sessionId);
    if (uniqueId >= 0)
      m_query->addBindValue(uniqueId);

    return m_query->exec();
  }

  /**
   * @brief Yields the next reading, or false once the session is exhausted.
   */
  [[nodiscard]] bool next(ReadingRow& out)
  {
    if (!m_query)
      return false;

    if (!m_blocks)
      return nextLegacy(out);

    // code-verify off
    // Bounded: each pass either serves a buffered row or decodes one more block, and the query
    // is finite.
    while (m_cursor >= m_buffer.size()) {
      m_buffer.clear();
      m_cursor = 0;
      if (!m_query->next())
        return false;

      if (!decodeBlockRow(*m_query, m_buffer)) {
        m_damaged = true;
        return false;
      }
    }
    // code-verify on

    out = std::move(m_buffer[m_cursor++]);
    return true;
  }

  /**
   * @brief Whether the walk stopped on a malformed block rather than at the end of the session.
   *        A consumer that cannot tell these apart reports archive damage as a value mismatch.
   */
  [[nodiscard]] bool damaged() const noexcept { return m_damaged; }

private:
  /**
   * @brief Serves one row straight from the legacy `readings` result set.
   */
  [[nodiscard]] bool nextLegacy(ReadingRow& out)
  {
    if (!m_query->next())
      return false;

    out.timestampNs  = m_query->value(0).toLongLong();
    out.uniqueId     = m_query->value(1).toInt();
    out.rawNumeric   = SerialStudio::toDouble(m_query->value(2));
    out.rawString    = m_query->value(3).toString();
    out.finalNumeric = SerialStudio::toDouble(m_query->value(4));
    out.finalString  = m_query->value(5).toString();
    out.isNumeric    = m_query->value(6).toInt() != 0;
    return true;
  }

private:
  bool m_blocks        = false;
  bool m_damaged       = false;
  std::size_t m_cursor = 0;
  std::vector<ReadingRow> m_buffer;
  std::optional<QSqlQuery> m_query;
};

}  // namespace Sessions

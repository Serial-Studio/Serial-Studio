/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "CSV/PlayerLoaderWorker.h"

#include <algorithm>
#include <cmath>
#include <QDateTime>

#include "Core/DSPSimd.h"
#include "Core/SSAssert.h"
#include "SerialStudio.h"

static constexpr qsizetype kBatchRows = 65536;

//--------------------------------------------------------------------------------------------------
// Legacy date/time parsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses an unsigned integer field of exactly @p digits digits at @p i, advancing it;
 *        exact widths keep the fast scanner a strict subset of the fixed-format fallback.
 */
[[nodiscard]] static bool parseIntField(QByteArrayView cell, qsizetype& i, int digits, int& out)
{
  SS_ASSERT(digits > 0, return false);
  SS_ASSERT(i >= 0, return false);

  int parsed = 0;
  int value  = 0;
  for (; i < cell.size() && parsed < digits; ++parsed, ++i) {
    const char c = cell.at(i);
    if (c < '0' || c > '9')
      break;

    value = value * 10 + (c - '0');
  }

  if (parsed != digits)
    return false;

  out = value;
  return true;
}

/**
 * @brief Consumes one literal character at @p i, advancing it on match.
 */
[[nodiscard]] static bool parseLiteral(QByteArrayView cell, qsizetype& i, char literal)
{
  if (i >= cell.size() || cell.at(i) != literal)
    return false;

  ++i;
  return true;
}

/**
 * @brief Fast scanner for the four legacy formats "yyyy/MM/dd[/] HH:mm:ss[::zzz]"; falls back
 *        to QDateTime::fromString for anything it cannot consume, so acceptance matches the
 *        old QString path exactly. Returns local-time epoch milliseconds.
 */
bool CSV::parseLegacyDateTimeMs(QByteArrayView cell, qint64& msOut)
{
  qsizetype i = 0;
  int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0, milli = 0;

  const bool date_ok = parseIntField(cell, i, 4, year) && parseLiteral(cell, i, '/')
                    && parseIntField(cell, i, 2, month) && parseLiteral(cell, i, '/')
                    && parseIntField(cell, i, 2, day);

  bool fast_ok = date_ok;
  if (fast_ok && i < cell.size() && cell.at(i) == '/')
    ++i;

  fast_ok = fast_ok && parseLiteral(cell, i, ' ') && parseIntField(cell, i, 2, hour)
         && parseLiteral(cell, i, ':') && parseIntField(cell, i, 2, minute)
         && parseLiteral(cell, i, ':') && parseIntField(cell, i, 2, second);

  if (fast_ok && i < cell.size()) {
    fast_ok = parseLiteral(cell, i, ':') && parseLiteral(cell, i, ':')
           && parseIntField(cell, i, 3, milli) && i == cell.size();
  }

  if (fast_ok) {
    const QDateTime dt(QDate(year, month, day), QTime(hour, minute, second, milli));
    if (dt.isValid()) {
      msOut = dt.toMSecsSinceEpoch();
      return true;
    }
  }

  static const QStringList kFormats = {QStringLiteral("yyyy/MM/dd HH:mm:ss::zzz"),
                                       QStringLiteral("yyyy/MM/dd/ HH:mm:ss::zzz"),
                                       QStringLiteral("yyyy/MM/dd HH:mm:ss"),
                                       QStringLiteral("yyyy/MM/dd/ HH:mm:ss")};

  const QString text = QString::fromUtf8(cell);
  for (const auto& format : kFormats) {
    const QDateTime dt = QDateTime::fromString(text, format);
    if (dt.isValid()) {
      msOut = dt.toMSecsSinceEpoch();
      return true;
    }
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Worker
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the loader worker (thread affinity is assigned by the player).
 */
CSV::PlayerLoaderWorker::PlayerLoaderWorker(QObject* parent)
  : QObject(parent), m_cancelRequested(false), m_rowLimitHit(false), m_validRows(0)
{}

/**
 * @brief Requests cooperative cancellation; the scan loop checks this between rows.
 */
void CSV::PlayerLoaderWorker::requestCancel()
{
  m_cancelRequested.store(true, std::memory_order_relaxed);
}

/**
 * @brief Computes one valid row's seconds according to the request's timestamp mode, mirroring
 *        the legacy caches: raw numeric values, anchored date/time deltas, index * interval,
 *        or -1 when the row carries no usable time.
 */
double CSV::PlayerLoaderWorker::secondsForRow(const PlayerIndexRequest& request,
                                              const DataModel::ReplayCellViews& cells) const
{
  SS_ASSERT(!cells.isEmpty(), return -1.0);

  switch (request.mode) {
    case PlayerTimestampMode::Interval:
      return static_cast<double>(m_validRows) * request.intervalSeconds;

    case PlayerTimestampMode::Numeric: {
      bool ok            = false;
      const double value = SerialStudio::toDouble(cells.first(), &ok);
      return (ok && value >= 0.0 && std::isfinite(value)) ? value * request.timeScale : -1.0;
    }

    case PlayerTimestampMode::DateTime:
    case PlayerTimestampMode::DateTimeColumn: {
      const qsizetype column =
        (request.mode == PlayerTimestampMode::DateTime) ? 0 : request.timestampColumn;
      if (column < 0 || column >= cells.size())
        return -1.0;

      qint64 ms = 0;
      if (!parseLegacyDateTimeMs(cells.at(column), ms))
        return -1.0;

      return static_cast<double>(ms - request.anchorMsSinceEpoch) * 0.001;
    }
  }

  return -1.0;
}

/**
 * @brief Splits one raw row, applies the legacy validity rule (any non-empty cell) and appends
 *        the row's offset + seconds to the current batch.
 */
void CSV::PlayerLoaderWorker::processRow(const PlayerIndexRequest& request,
                                         qint64 begin,
                                         qint64 end,
                                         DataModel::ReplayCellViews& cells,
                                         QByteArray& scratch,
                                         PlayerIndexBatch& batch)
{
  SS_ASSERT(begin >= 0, return);

  if (begin >= end)
    return;

  if (end - begin > kMaxCsvRowBytes) [[unlikely]]
    return;

  if (m_validRows >= kMaxIndexedRows) [[unlikely]] {
    m_rowLimitHit = true;
    return;
  }

  DataModel::splitReplayRowSpans(
    QByteArrayView(request.data + begin, static_cast<qsizetype>(end - begin)),
    cells,
    scratch,
    request.separator);

  const bool valid = std::any_of(
    cells.cbegin(), cells.cend(), [](const QByteArrayView& cell) { return !cell.isEmpty(); });
  if (!valid)
    return;

  quint8 bits          = 0;
  const qsizetype bitN = request.fileColumnSourceBit.size();
  for (qsizetype c = 0; c < cells.size() && c < bitN; ++c)
    if (!cells.at(c).isEmpty())
      bits |= request.fileColumnSourceBit.at(c);

  batch.rowOffsets.append(static_cast<quint64>(begin));
  batch.rowSeconds.append(secondsForRow(request, cells));
  batch.rowSourceBits.append(bits);
  ++m_validRows;
}

/**
 * @brief Indexes the mapped file: SIMD newline scan from the data offset, per-row validity +
 *        seconds, batched emission, cooperative cancel. Emits finished(false) when cancelled.
 */
void CSV::PlayerLoaderWorker::indexFile(const CSV::PlayerIndexRequestPtr& request)
{
  SS_ASSERT(request != nullptr, return);
  SS_ASSERT(request->data != nullptr || request->size == 0, {
    Q_EMIT finished(false, request->generation);
    return;
  });
  SS_ASSERT(request->dataOffset >= 0 && request->dataOffset <= request->size, {
    Q_EMIT finished(false, request->generation);
    return;
  });

  m_validRows = 0;

  DataModel::ReplayCellViews cells;
  QByteArray scratch;
  auto batch = std::make_shared<PlayerIndexBatch>();

  constexpr qint64 kScanChunkBytes = 4 * 1024 * 1024;

  const qint64 size = request->size;
  const char* data  = request->data;
  qint64 rowStart   = request->dataOffset;
  bool stopped      = false;

  for (qint64 chunkStart  = request->dataOffset; chunkStart < size && !stopped;
       chunkStart        += kScanChunkBytes) {
    const qint64 chunkLen = qMin(kScanChunkBytes, size - chunkStart);
    const bool scanned =
      DSP::simdForEachByteMatch(data + chunkStart, chunkLen, '\n', [&](qsizetype pos) {
        const qint64 rowEnd = chunkStart + pos;
        processRow(*request, rowStart, rowEnd, cells, scratch, *batch);
        rowStart = rowEnd + 1;

        if (batch->rowOffsets.size() >= kBatchRows) {
          batch->bytesIndexed = rowStart;
          batch->generation   = request->generation;
          Q_EMIT batchReady(batch);
          batch = std::make_shared<PlayerIndexBatch>();
        }

        return !m_cancelRequested.load(std::memory_order_relaxed) && !m_rowLimitHit;
      });

    stopped = !scanned || m_cancelRequested.load(std::memory_order_relaxed) || m_rowLimitHit;
  }

  if (m_cancelRequested.load(std::memory_order_relaxed)) {
    Q_EMIT finished(false, request->generation);
    return;
  }

  if (!m_rowLimitHit)
    processRow(*request, rowStart, size, cells, scratch, *batch);

  if (m_rowLimitHit) [[unlikely]]
    qWarning() << "[CSV::PlayerLoaderWorker] Row limit reached (" << kMaxIndexedRows
               << "); file indexed partially.";

  batch->bytesIndexed = m_rowLimitHit ? rowStart : size;
  batch->generation   = request->generation;
  Q_EMIT batchReady(batch);
  Q_EMIT finished(true, request->generation);
}

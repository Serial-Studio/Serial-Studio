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

#pragma once

#include <atomic>
#include <memory>
#include <QByteArray>
#include <QObject>
#include <QVector>

#include "DataModel/Scripting/FrameParserPipeline.h"

namespace CSV {

/**
 * @brief Longest row the indexer accepts; a "row" beyond this (e.g. a newline-free binary
 *        blob renamed .csv) is treated as invalid data instead of split in one piece.
 */
inline constexpr qint64 kMaxCsvRowBytes = 4 * 1024 * 1024;

/**
 * @brief Hard ceiling on indexed rows so frame positions stay within int range; hitting it
 *        stops indexing and is reported to the user (spec 0022 R4: never silent).
 */
inline constexpr qsizetype kMaxIndexedRows = 2'000'000'000;

/**
 * @brief Parses one cell in the four legacy date/time formats (fast fixed-format scanner
 *        with a QDateTime::fromString fallback); returns local-time epoch milliseconds.
 */
[[nodiscard]] bool parseLegacyDateTimeMs(QByteArrayView cell, qint64& msOut);

/**
 * @brief How the indexer derives per-row seconds; detected by the player's foreground quick
 *        pass before the worker starts.
 */
enum class PlayerTimestampMode {
  Numeric,
  DateTime,
  Interval,
  DateTimeColumn,
};

/**
 * @brief One background-indexing request: a borrowed view of the mapped file plus the
 *        timestamp mode. The player guarantees the mapped bytes outlive the worker run
 *        (cancel + join before unmapping).
 */
struct PlayerIndexRequest {
  const char* data          = nullptr;
  qint64 size               = 0;
  qint64 dataOffset         = 0;
  int timestampColumn       = 0;
  double intervalSeconds    = 0.0;
  qint64 anchorMsSinceEpoch = 0;
  quint64 generation        = 0;
  PlayerTimestampMode mode  = PlayerTimestampMode::Numeric;
};

/**
 * @brief Shared pointer alias for PlayerIndexRequest, exchanged across threads.
 */
using PlayerIndexRequestPtr = std::shared_ptr<PlayerIndexRequest>;

/**
 * @brief One batch of indexed rows: absolute byte offsets of valid rows and their seconds
 *        (-1 = no usable time), plus the cumulative byte position for progress display.
 */
struct PlayerIndexBatch {
  QVector<quint64> rowOffsets;
  QVector<double> rowSeconds;
  qint64 bytesIndexed = 0;
  quint64 generation  = 0;
};

/**
 * @brief Shared pointer alias for PlayerIndexBatch, exchanged across threads.
 */
using PlayerIndexBatchPtr = std::shared_ptr<PlayerIndexBatch>;

/**
 * @brief Worker that indexes a mapped CSV recording off the main thread: newline scan,
 *        row-validity filter and per-row seconds, batched back to the player.
 */
class PlayerLoaderWorker : public QObject {
  Q_OBJECT

signals:
  void batchReady(const CSV::PlayerIndexBatchPtr& batch);
  void finished(bool ok, quint64 generation);

public:
  explicit PlayerLoaderWorker(QObject* parent = nullptr);

  PlayerLoaderWorker(PlayerLoaderWorker&&)                 = delete;
  PlayerLoaderWorker(const PlayerLoaderWorker&)            = delete;
  PlayerLoaderWorker& operator=(PlayerLoaderWorker&&)      = delete;
  PlayerLoaderWorker& operator=(const PlayerLoaderWorker&) = delete;

  void requestCancel();

public slots:
  void indexFile(const CSV::PlayerIndexRequestPtr& request);

private:
  void processRow(const PlayerIndexRequest& request,
                  qint64 begin,
                  qint64 end,
                  DataModel::ReplayCellViews& cells,
                  QByteArray& scratch,
                  PlayerIndexBatch& batch);
  [[nodiscard]] double secondsForRow(const PlayerIndexRequest& request,
                                     const DataModel::ReplayCellViews& cells) const;

private:
  std::atomic<bool> m_cancelRequested;
  bool m_rowLimitHit;
  qsizetype m_validRows;
};

}  // namespace CSV

Q_DECLARE_METATYPE(CSV::PlayerIndexRequestPtr)
Q_DECLARE_METATYPE(CSV::PlayerIndexBatchPtr)

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

#ifdef BUILD_COMMERCIAL

#  include "Sessions/Player/SessionDbReader.h"

#  include <QDateTime>
#  include <QDebug>
#  include <QSqlError>

#  include "Core/SSAssert.h"
#  include "SerialStudio.h"
#  include "Sessions/BlockReader.h"
#  include "Sessions/StreamBlockCodec.h"

//--------------------------------------------------------------------------------------------------
// Connection lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a closed reader; no connection exists until open() succeeds.
 */
Sessions::SessionDbReader::SessionDbReader()
  : m_sessionId(-1)
  , m_usesBlocks(false)
  , m_hasFinalValues(false)
  , m_seekQueryPrepared(false)
  , m_frameQueryPrepared(false)
{}

/**
 * @brief Releases the connection and its prepared statements.
 */
Sessions::SessionDbReader::~SessionDbReader()
{
  close();
}

/**
 * @brief Returns @c true while a session file is open for reading.
 */
bool Sessions::SessionDbReader::isOpen() const
{
  return m_db && m_db->isOpen();
}

/**
 * @brief Returns the session the reader answers for, or -1.
 */
int Sessions::SessionDbReader::sessionId() const noexcept
{
  return m_sessionId;
}

/**
 * @brief Returns @c true when the session stores its samples as spec-0055 blocks.
 */
bool Sessions::SessionDbReader::usesBlocks() const noexcept
{
  return m_usesBlocks;
}

/**
 * @brief Returns @c true when post-transform values are stored alongside the raw ones.
 */
bool Sessions::SessionDbReader::hasFinalValues() const noexcept
{
  return m_hasFinalValues;
}

/**
 * @brief Opens a main-thread connection to @p filePath for @p sessionId. The session id is taken
 *        here rather than set later because the schema probe below already queries by it: a stale
 *        id made a block-format session read the empty readings table for a whole replay
 *        (spec 0064).
 */
bool Sessions::SessionDbReader::open(const QString& filePath, int sessionId)
{
  SS_ASSERT(!filePath.isEmpty(), return false);
  SS_ASSERT(!m_db.has_value(), close());

  m_sessionId      = sessionId;
  m_connectionName = QStringLiteral("ss_sqlite_player_%1").arg(QDateTime::currentMSecsSinceEpoch());

  m_db.emplace(QSqlDatabase::addDatabase("QSQLITE", m_connectionName));
  m_db->setDatabaseName(filePath);

  // code-verify off
  // Replay never writes, and journal_mode=WAL IS a write: forcing it made an archive on read-only
  // media (a mounted image, a locked share, a burned copy) unplayable (B15).
  // code-verify on
  m_db->setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
  if (!m_db->open()) {
    close();
    return false;
  }

  QSqlQuery pragma(*m_db);
  pragma.exec("PRAGMA busy_timeout=5000");

  detectFinalValueColumns();
  return true;
}

/**
 * @brief Closes the connection, drops every prepared statement and forgets the session. The
 *        statements go first: a prepared handle outliving its connection is a use-after-free.
 */
void Sessions::SessionDbReader::close()
{
  if (m_frameQuery)
    m_frameQuery->clear();

  if (m_seekQuery)
    m_seekQuery->clear();

  m_frameQuery.reset();
  m_seekQuery.reset();
  m_streamBlobQuery.reset();
  m_denseBlobQuery.reset();
  m_frameQueryPrepared = false;
  m_seekQueryPrepared  = false;

  if (m_db && m_db->isOpen())
    m_db->close();

  const QString conn = m_connectionName;
  m_db.reset();
  if (!conn.isEmpty())
    QSqlDatabase::removeDatabase(conn);

  m_connectionName.clear();
  m_sessionId      = -1;
  m_usesBlocks     = false;
  m_hasFinalValues = false;
}

/**
 * @brief Probes the storage format: block sessions always carry final values, older readings
 *        tables only do when the migration that added the columns has run.
 */
void Sessions::SessionDbReader::detectFinalValueColumns()
{
  SS_ASSERT(m_db && m_db->isOpen(), return);

  m_usesBlocks = Sessions::sessionUsesBlocks(*m_db, m_sessionId);
  if (m_usesBlocks) {
    m_hasFinalValues = true;
    return;
  }

  m_hasFinalValues = false;

  QSqlQuery probe(*m_db);
  if (!probe.exec("PRAGMA table_info(readings)"))
    return;

  while (probe.next()) {
    if (probe.value(1).toString() == QLatin1String("final_numeric_value")) {
      m_hasFinalValues = true;
      return;
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Cursor row
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the cells stored at @p timestampNs, from whichever table this session uses.
 */
Sessions::ReplayRowValues Sessions::SessionDbReader::readFrameValues(qint64 timestampNs,
                                                                     const ReplayLayout& layout)
{
  if (!m_db) [[unlikely]]
    return {};

  if (m_usesBlocks)
    return readRowFromBlocks(timestampNs, layout);

  return readRowFromReadings(timestampNs, layout);
}

/**
 * @brief Reads one readings row set. Replays the stored final (post-transform) values; the raw
 *        columns are the fallback for files written before the final ones existed.
 */
Sessions::ReplayRowValues Sessions::SessionDbReader::readRowFromReadings(qint64 timestampNs,
                                                                         const ReplayLayout& layout)
{
  ReplayRowValues out;
  out.values.reserve(static_cast<int>(layout.columnUniqueIds.size()));

  if (!m_frameQueryPrepared) {
    m_frameQuery.emplace(*m_db);
    m_frameQuery->setForwardOnly(true);
    const auto query =
      m_hasFinalValues
        ? QStringLiteral("SELECT unique_id, final_numeric_value, final_string_value, is_numeric "
                         "FROM readings WHERE session_id = ? AND timestamp_ns = ? "
                         "ORDER BY reading_id")
        : QStringLiteral("SELECT unique_id, raw_numeric_value, raw_string_value, is_numeric "
                         "FROM readings WHERE session_id = ? AND timestamp_ns = ? "
                         "ORDER BY reading_id");
    m_frameQuery->prepare(query);
    m_frameQueryPrepared = true;
  }

  m_frameQuery->bindValue(0, m_sessionId);
  m_frameQuery->bindValue(1, timestampNs);

  if (!m_frameQuery->exec()) [[unlikely]] {
    qWarning() << "[Sessions::SessionDbReader] frame query failed:"
               << m_frameQuery->lastError().text();
    return out;
  }

  while (m_frameQuery->next()) {
    ReadingRow row;
    row.timestampNs  = timestampNs;
    row.uniqueId     = m_frameQuery->value(0).toInt();
    row.isNumeric    = m_frameQuery->value(3).toInt() != 0;
    row.finalNumeric = SerialStudio::toDouble(m_frameQuery->value(1));
    row.finalString  = m_frameQuery->value(2).toString();
    ReplayFrameValues::applyRow(out, row, layout);
  }

  m_frameQuery->finish();
  return out;
}

/**
 * @brief Spec-0055 twin of the cursor-row read: the irregular blocks containing @p timestampNs are
 *        those whose indexed span covers it, and the sample at that exact instant is the replayed
 *        cell. Dense rows (dt_ns != 0) are excluded: they replay whole through the stream-block
 *        lane (R11), and injecting their t0 sample here would publish it twice.
 */
Sessions::ReplayRowValues Sessions::SessionDbReader::readRowFromBlocks(qint64 timestampNs,
                                                                       const ReplayLayout& layout)
{
  ReplayRowValues out;

  QSqlQuery q(*m_db);
  q.setForwardOnly(true);
  q.prepare(QStringLiteral("SELECT %1 FROM blocks WHERE session_id = ? AND dt_ns = 0 "
                           "AND t0_ns <= ? AND t_end_ns >= ? ORDER BY block_id")
              .arg(QLatin1String(Sessions::kBlockColumns)));
  q.bindValue(0, m_sessionId);
  q.bindValue(1, timestampNs);
  q.bindValue(2, timestampNs);

  if (!q.exec()) [[unlikely]] {
    qWarning() << "[Sessions::SessionDbReader] block frame query failed:" << q.lastError().text();
    return out;
  }

  std::vector<Sessions::ReadingRow> rows;
  while (q.next()) {
    rows.clear();
    if (!Sessions::decodeBlockRow(q, rows))
      continue;

    ReplayFrameValues::selectRowsAt(out, rows, timestampNs, layout);
  }

  q.finish();
  return out;
}

//--------------------------------------------------------------------------------------------------
// Seek window
//--------------------------------------------------------------------------------------------------

/**
 * @brief Fills the per-series numeric window covering @p rowTimes, from whichever table this
 *        session uses. Sparse readings forward-fill, and leading gaps backfill from the first
 *        stored value, so a scrub shows the step function the recording captured.
 */
void Sessions::SessionDbReader::fillSeekWindow(std::span<const qint64> rowTimes,
                                               const QHash<int, qint64>& keyByUid,
                                               QHash<qint64, QVector<double>>& series)
{
  SS_ASSERT(!rowTimes.empty(), return);

  if (!m_db) [[unlikely]]
    return;

  if (m_usesBlocks)
    fillSeekWindowFromBlocks(rowTimes, keyByUid, series);
  else
    fillSeekWindowFromReadings(rowTimes, keyByUid, series);

  for (auto it = series.begin(); it != series.end(); ++it)
    ReplayFrameValues::fillSeekGaps(it.value());
}

/**
 * @brief One windowed range query over readings (covering index; ties broken by reading_id),
 *        dropping each stored value onto the window row whose timestamp matches it exactly.
 */
void Sessions::SessionDbReader::fillSeekWindowFromReadings(std::span<const qint64> rowTimes,
                                                           const QHash<int, qint64>& keyByUid,
                                                           QHash<qint64, QVector<double>>& series)
{
  if (!m_seekQueryPrepared) {
    m_seekQuery.emplace(*m_db);
    m_seekQuery->setForwardOnly(true);
    const bool prepared = m_seekQuery->prepare(
      m_hasFinalValues ? QStringLiteral("SELECT unique_id, final_numeric_value, timestamp_ns "
                                        "FROM readings WHERE session_id = ? AND timestamp_ns "
                                        "BETWEEN ? AND ? ORDER BY timestamp_ns, reading_id")
                       : QStringLiteral("SELECT unique_id, raw_numeric_value, timestamp_ns "
                                        "FROM readings WHERE session_id = ? AND timestamp_ns "
                                        "BETWEEN ? AND ? ORDER BY timestamp_ns, reading_id"));
    if (!prepared) [[unlikely]] {
      qWarning() << "[Sessions::SessionDbReader] seek query prepare failed:"
                 << m_seekQuery->lastError().text();
      m_seekQuery.reset();
      series.clear();
      return;
    }

    m_seekQueryPrepared = true;
  }

  m_seekQuery->bindValue(0, m_sessionId);
  m_seekQuery->bindValue(1, rowTimes.front());
  m_seekQuery->bindValue(2, rowTimes.back());

  if (!m_seekQuery->exec()) [[unlikely]] {
    qWarning() << "[Sessions::SessionDbReader] seek window query failed:"
               << m_seekQuery->lastError().text();
    series.clear();
    return;
  }

  std::vector<Sessions::ReadingRow> rows(1);
  while (m_seekQuery->next()) {
    rows[0].uniqueId     = m_seekQuery->value(0).toInt();
    rows[0].finalNumeric = SerialStudio::toDouble(m_seekQuery->value(1));
    rows[0].timestampNs  = m_seekQuery->value(2).toLongLong();
    ReplayFrameValues::scatterRowsIntoWindow(rows, rowTimes, keyByUid, series);
  }

  m_seekQuery->finish();
}

/**
 * @brief Spec-0055 twin of the seek window: selects the blocks overlapping the window by their
 *        indexed [t0_ns, t_end_ns] span, decodes them, and drops each sample onto its row. The
 *        span index is what keeps this a lookup rather than a decode of the whole session.
 */
void Sessions::SessionDbReader::fillSeekWindowFromBlocks(std::span<const qint64> rowTimes,
                                                         const QHash<int, qint64>& keyByUid,
                                                         QHash<qint64, QVector<double>>& series)
{
  QSqlQuery q(*m_db);
  q.setForwardOnly(true);
  q.prepare(QStringLiteral("SELECT %1 FROM blocks WHERE session_id = ? AND t_end_ns >= ? "
                           "AND t0_ns <= ? ORDER BY t0_ns, block_id")
              .arg(QLatin1String(Sessions::kBlockColumns)));
  q.bindValue(0, m_sessionId);
  q.bindValue(1, rowTimes.front());
  q.bindValue(2, rowTimes.back());

  if (!q.exec()) [[unlikely]] {
    qWarning() << "[Sessions::SessionDbReader] block seek query failed:" << q.lastError().text();
    series.clear();
    return;
  }

  std::vector<Sessions::ReadingRow> rows;
  while (q.next()) {
    rows.clear();
    if (!Sessions::decodeBlockRow(q, rows))
      continue;

    ReplayFrameValues::scatterRowsIntoWindow(rows, rowTimes, keyByUid, series);
  }

  q.finish();
}

//--------------------------------------------------------------------------------------------------
// Dense sample blobs
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads one dense block's samples blob by rowid -- from `stream_blocks` (spec 0054) or
 *        `blocks` (spec 0055) per the entry's tag -- and decodes it from canonical little-endian
 *        float64. Rejects a blob whose length is not `frames * 8` rather than decoding past its
 *        end -- a truncated or foreign file must fail loudly, not silently misplay.
 */
bool Sessions::SessionDbReader::fetchStreamSamples(const PlayerStreamBlockIndex& entry,
                                                   std::vector<double>& out)
{
  SS_ASSERT(entry.frames >= 0, return false);

  if (!m_db || !m_db->isOpen()) [[unlikely]]
    return false;

  auto& blobQuery = entry.fromBlocks ? m_denseBlobQuery : m_streamBlobQuery;
  if (!blobQuery) {
    blobQuery.emplace(*m_db);
    blobQuery->setForwardOnly(true);
    blobQuery->prepare(entry.fromBlocks
                         ? QStringLiteral("SELECT values_blob FROM blocks WHERE block_id = ?")
                         : QStringLiteral("SELECT samples FROM stream_blocks "
                                          "WHERE stream_block_id = ?"));
  }

  blobQuery->bindValue(0, entry.rowId);
  if (!blobQuery->exec() || !blobQuery->next()) [[unlikely]] {
    qWarning() << "[Sessions::SessionDbReader] stream block fetch failed:"
               << blobQuery->lastError().text();
    return false;
  }

  const QByteArray blob = blobQuery->value(0).toByteArray();
  blobQuery->finish();

  if (!unpackStreamSamples(blob, entry.frames, out)) [[unlikely]] {
    qWarning() << "[Sessions::SessionDbReader] stream block" << entry.rowId << "has" << blob.size()
               << "bytes, expected" << (entry.frames * kStreamSampleBytes);
    return false;
  }

  return true;
}

#endif

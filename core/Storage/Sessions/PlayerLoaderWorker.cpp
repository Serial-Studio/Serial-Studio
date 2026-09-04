/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "Sessions/PlayerLoaderWorker.h"

#  include <algorithm>
#  include <QDateTime>
#  include <QDebug>
#  include <QObject>
#  include <QSqlDatabase>
#  include <QSqlError>
#  include <QSqlQuery>
#  include <tuple>

#  include "Sessions/BlockReader.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initializes the cancellation flag.
 */
Sessions::PlayerLoaderWorker::PlayerLoaderWorker(QObject* parent)
  : QObject(parent), m_cancelRequested(false)
{}

/**
 * @brief Trivial destructor -- no owned resources outside per-call scope.
 */
Sessions::PlayerLoaderWorker::~PlayerLoaderWorker() = default;

//--------------------------------------------------------------------------------------------------
// Cancellation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the atomic cancel flag; the load loop checks it between rows.
 */
void Sessions::PlayerLoaderWorker::requestCancel()
{
  m_cancelRequested.store(true, std::memory_order_release);
}

//--------------------------------------------------------------------------------------------------
// Worker entry point
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resolves the latest session id when the caller asked for -1.
 */
bool Sessions::PlayerLoaderWorker::resolveSessionId(QSqlDatabase& db,
                                                    int& sessionId,
                                                    QString& errorOut)
{
  if (sessionId >= 0)
    return true;

  QSqlQuery q(db);
  q.prepare("SELECT session_id FROM sessions ORDER BY started_at DESC LIMIT 1");
  if (!q.exec() || !q.next()) {
    errorOut = tr("This file does not contain any recording sessions.");
    return false;
  }

  sessionId = q.value(0).toInt();
  return true;
}

/**
 * @brief Loads the embedded project JSON for the session (falling back to the global row) and the
 *        spec-0062 view-state bundle (absent on pre-0062 archives, the query then yields NULL).
 */
void Sessions::PlayerLoaderWorker::loadProjectJson(QSqlDatabase& db,
                                                   int sessionId,
                                                   PlayerSessionPayload& payload)
{
  {
    QSqlQuery q(db);
    q.prepare("SELECT project_json FROM sessions WHERE session_id = ?");
    q.bindValue(0, sessionId);
    if (q.exec() && q.next())
      payload.projectJson = q.value(0).toString();
  }

  {
    QSqlQuery q(db);
    q.prepare("SELECT view_state FROM sessions WHERE session_id = ?");
    q.bindValue(0, sessionId);
    if (q.exec() && q.next())
      payload.viewState = q.value(0).toString();
  }

  if (payload.projectJson.isEmpty()) {
    QSqlQuery q(db);
    q.prepare("SELECT value FROM project_metadata WHERE key = 'project_json'");
    if (q.exec() && q.next())
      payload.projectJson = q.value(0).toString();
  }
}

/**
 * @brief Loads the unique-id column order that the live FrameBuilder must mirror.
 */
bool Sessions::PlayerLoaderWorker::loadColumnOrder(QSqlDatabase& db,
                                                   int sessionId,
                                                   PlayerSessionPayload& payload,
                                                   QString& errorOut)
{
  QSqlQuery q(db);
  q.setForwardOnly(true);
  q.prepare("SELECT unique_id FROM columns WHERE session_id = ? ORDER BY column_id ASC");
  q.bindValue(0, sessionId);
  if (!q.exec()) {
    errorOut = q.lastError().text();
    return false;
  }

  while (q.next())
    payload.columnUniqueIds.push_back(q.value(0).toInt());

  return true;
}

/**
 * @brief Loads the distinct frame timestamps for the session, with periodic cancel checks.
 */
bool Sessions::PlayerLoaderWorker::loadTimestampIndex(QSqlDatabase& db,
                                                      int sessionId,
                                                      PlayerSessionPayload& payload,
                                                      QString& errorOut)
{
  if (Sessions::sessionUsesBlocks(db, sessionId))
    return loadBlockTimestampIndex(db, sessionId, payload, errorOut);

  QSqlQuery q(db);
  q.setForwardOnly(true);
  q.prepare("SELECT DISTINCT timestamp_ns FROM readings "
            "WHERE session_id = ? ORDER BY timestamp_ns ASC");
  q.bindValue(0, sessionId);
  if (!q.exec()) {
    errorOut = q.lastError().text();
    return false;
  }

  qint64 row = 0;
  while (q.next()) {
    if ((row & 0xFFFF) == 0 && m_cancelRequested.load(std::memory_order_acquire)) {
      errorOut = tr("Cancelled");
      return false;
    }

    payload.timestampsNs.push_back(q.value(0).toLongLong());
    ++row;
  }

  return true;
}

/**
 * @brief Collects the dense (uniform-grid) blocks of a session: one stream-block entry per dataset
 *        row, and each block's start instant ONCE. A dense block contributes only its start (48 kHz
 *        would otherwise materialise ~29 M timestamps per 10 minutes) and joins the stream-block
 *        index instead (R11), so playback replays it whole through the sink-masked block lane.
 */
[[nodiscard]] static bool loadDenseBlocks(QSqlDatabase& db,
                                          int sessionId,
                                          Sessions::PlayerSessionPayload& payload,
                                          const std::atomic<bool>& cancelled,
                                          QString& errorOut)
{
  QSqlQuery q(db);
  q.setForwardOnly(true);
  q.prepare("SELECT block_id, source_id, unique_id, t0_ns, dt_ns, frames FROM blocks "
            "WHERE session_id = ? AND dt_ns != 0 ORDER BY t0_ns ASC, block_id ASC");
  q.bindValue(0, sessionId);
  if (!q.exec()) {
    errorOut = q.lastError().text();
    return false;
  }

  qint64 row = 0;
  while (q.next()) {
    if ((row & 0xFF) == 0 && cancelled.load(std::memory_order_acquire)) {
      errorOut = QObject::tr("Cancelled");
      return false;
    }

    ++row;
    const qint64 t0Ns = q.value(3).toLongLong();
    if (payload.timestampsNs.empty() || payload.timestampsNs.back() != t0Ns)
      payload.timestampsNs.push_back(t0Ns);

    const qint64 frames = q.value(5).toLongLong();
    if (frames <= 0 || frames > Sessions::kMaxBlockFrames) {
      qWarning() << "[Sessions::PlayerLoader] skipping dense block" << q.value(0).toLongLong()
                 << "with invalid frame count" << frames;
      continue;
    }

    Sessions::PlayerStreamBlockIndex entry;
    entry.rowId      = q.value(0).toLongLong();
    entry.sourceId   = q.value(1).toInt();
    entry.uniqueId   = q.value(2).toInt();
    entry.t0Ns       = t0Ns;
    entry.dtNs       = q.value(4).toLongLong();
    entry.frames     = frames;
    entry.fromBlocks = true;
    payload.streamBlocks.push_back(entry);
  }

  return true;
}

/**
 * @brief Expands the irregular blocks' per-sample instants ONCE PER BLOCK. The `blocks` table
 *        holds one row per dataset per block, all repeating the same times blob, so expanding per
 *        row cost 635 copies of every instant in a 635-dataset project -- 1.8 GB transient for an
 *        hour at 100 Hz (B7). (source_id, block_number) is the block's identity.
 */
[[nodiscard]] static bool loadIrregularBlockTimes(QSqlDatabase& db,
                                                  int sessionId,
                                                  Sessions::PlayerSessionPayload& payload,
                                                  const std::atomic<bool>& cancelled,
                                                  QString& errorOut)
{
  QSqlQuery q(db);
  q.setForwardOnly(true);
  q.prepare("SELECT t0_ns, dt_ns, frames, times FROM blocks "
            "WHERE session_id = ? AND dt_ns = 0 "
            "GROUP BY source_id, block_number ORDER BY t0_ns ASC");
  q.bindValue(0, sessionId);
  if (!q.exec()) {
    errorOut = q.lastError().text();
    return false;
  }

  std::vector<qint64> stamps;
  qint64 row = 0;
  while (q.next()) {
    if ((row & 0xFF) == 0 && cancelled.load(std::memory_order_acquire)) {
      errorOut = QObject::tr("Cancelled");
      return false;
    }

    ++row;
    stamps.clear();
    if (!Sessions::expandBlockTimes(q.value(0).toLongLong(),
                                    q.value(1).toLongLong(),
                                    q.value(2).toLongLong(),
                                    q.value(3).toByteArray(),
                                    stamps)) {
      errorOut = QObject::tr("Corrupt block timing in session %1").arg(sessionId);
      return false;
    }

    for (const qint64 stamp : stamps)
      payload.timestampsNs.push_back(stamp);
  }

  return true;
}

/**
 * @brief Spec-0055 twin of loadTimestampIndex: the dense blocks' starts plus every irregular
 *        block's sample instants, de-duplicated into one ascending index.
 */
bool Sessions::PlayerLoaderWorker::loadBlockTimestampIndex(QSqlDatabase& db,
                                                           int sessionId,
                                                           PlayerSessionPayload& payload,
                                                           QString& errorOut)
{
  if (!loadDenseBlocks(db, sessionId, payload, m_cancelRequested, errorOut))
    return false;

  if (!loadIrregularBlockTimes(db, sessionId, payload, m_cancelRequested, errorOut))
    return false;

  std::sort(payload.timestampsNs.begin(), payload.timestampsNs.end());
  payload.timestampsNs.erase(std::unique(payload.timestampsNs.begin(), payload.timestampsNs.end()),
                             payload.timestampsNs.end());

  return true;
}

/**
 * @brief Loads the stream-block index for the session: metadata only, ordered by time. The blobs
 *        stay on disk -- materializing them would scale replay memory with recording length
 *        (~23 MB/minute/channel), so playback fetches one block at a time (spec 0054). An archive
 *        predating the table indexes empty rather than failing to open.
 */
bool Sessions::PlayerLoaderWorker::loadStreamBlockIndex(QSqlDatabase& db,
                                                        int sessionId,
                                                        PlayerSessionPayload& payload,
                                                        QString& errorOut)
{
  if (!Sessions::archiveHasTable(db, QStringLiteral("stream_blocks")))
    return true;

  QSqlQuery q(db);
  q.setForwardOnly(true);
  q.prepare("SELECT stream_block_id, source_id, unique_id, t0_ns, dt_ns, frames "
            "FROM stream_blocks WHERE session_id = ? ORDER BY t0_ns ASC, stream_block_id ASC");
  q.bindValue(0, sessionId);
  if (!q.exec()) {
    errorOut = q.lastError().text();
    return false;
  }

  qint64 row = 0;
  while (q.next()) {
    if ((row & 0xFFFF) == 0 && m_cancelRequested.load(std::memory_order_acquire)) {
      errorOut = tr("Cancelled");
      return false;
    }

    PlayerStreamBlockIndex entry;
    entry.rowId      = q.value(0).toLongLong();
    entry.sourceId   = q.value(1).toInt();
    entry.uniqueId   = q.value(2).toInt();
    entry.t0Ns       = q.value(3).toLongLong();
    entry.dtNs       = q.value(4).toLongLong();
    entry.frames     = q.value(5).toLongLong();
    entry.fromBlocks = false;
    payload.streamBlocks.push_back(entry);
    ++row;
  }

  return true;
}

/**
 * @brief Opens an archive for the index load. Read-only by connect option and with no journal
 *        pragma: the load never writes, and journal_mode=WAL IS a write, which made an archive on
 *        read-only media (a mounted image, a locked share, a burned copy) unloadable (B15).
 */
[[nodiscard]] static QSqlDatabase openArchiveReadOnly(const QString& connectionName,
                                                      const QString& filePath)
{
  QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
  db.setDatabaseName(filePath);
  db.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
  if (!db.open())
    return db;

  QSqlQuery pragma(db);
  pragma.exec(QStringLiteral("PRAGMA busy_timeout=5000"));
  return db;
}

/**
 * @brief Opens the file, fetches column order + project JSON + timestamp index, ships it back.
 */
void Sessions::PlayerLoaderWorker::openAndLoad(const QString& filePath, int sessionId)
{
  m_cancelRequested.store(false, std::memory_order_release);

  auto payload       = std::make_shared<PlayerSessionPayload>();
  payload->ok        = false;
  payload->filePath  = filePath;
  payload->sessionId = sessionId;

  if (filePath.isEmpty()) {
    payload->error = tr("Empty file path");
    Q_EMIT loaded(payload);
    return;
  }

  const QString connName =
    QStringLiteral("ss_player_loader_%1").arg(QDateTime::currentMSecsSinceEpoch());

  auto closeAndEmit = [&](QSqlDatabase& db) {
    if (db.isOpen())
      db.close();

    db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connName);
    Q_EMIT loaded(payload);
  };

  QSqlDatabase db = openArchiveReadOnly(connName, filePath);
  if (!db.isOpen()) {
    payload->error = db.lastError().text();
    closeAndEmit(db);
    return;
  }

  if (!resolveSessionId(db, sessionId, payload->error)) {
    closeAndEmit(db);
    return;
  }
  payload->sessionId = sessionId;

  if (m_cancelRequested.load(std::memory_order_acquire)) {
    payload->error = tr("Cancelled");
    closeAndEmit(db);
    return;
  }

  loadProjectJson(db, sessionId, *payload);

  if (!loadColumnOrder(db, sessionId, *payload, payload->error)) {
    closeAndEmit(db);
    return;
  }

  if (payload->columnUniqueIds.empty()) {
    payload->error = tr("The selected session is missing its column definitions.");
    closeAndEmit(db);
    return;
  }

  if (m_cancelRequested.load(std::memory_order_acquire)) {
    payload->error = tr("Cancelled");
    closeAndEmit(db);
    return;
  }

  if (!loadTimestampIndex(db, sessionId, *payload, payload->error)) {
    closeAndEmit(db);
    return;
  }

  if (!loadStreamBlockIndex(db, sessionId, *payload, payload->error)) {
    closeAndEmit(db);
    return;
  }

  // code-verify off
  // The player's block walk groups consecutive entries by (t0, source); the two per-table ORDER BY
  // clauses guarantee neither cross-table order nor same-source contiguity at equal t0.
  // code-verify on
  std::stable_sort(payload->streamBlocks.begin(),
                   payload->streamBlocks.end(),
                   [](const PlayerStreamBlockIndex& a, const PlayerStreamBlockIndex& b) {
                     return std::tie(a.t0Ns, a.sourceId, a.rowId)
                          < std::tie(b.t0Ns, b.sourceId, b.rowId);
                   });

  if (payload->timestampsNs.empty() && payload->streamBlocks.empty()) {
    payload->error = tr("The selected session does not contain any frames.");
    closeAndEmit(db);
    return;
  }

  payload->ok = true;
  closeAndEmit(db);
}

#endif  // BUILD_COMMERCIAL

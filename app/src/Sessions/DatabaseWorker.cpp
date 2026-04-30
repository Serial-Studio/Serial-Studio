/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "Sessions/DatabaseWorker.h"

#  include <QDateTime>
#  include <QFile>
#  include <QFileInfo>
#  include <QMap>
#  include <QSqlError>
#  include <QSqlQuery>
#  include <QStringList>
#  include <QTextStream>
#  include <QThread>

#  include "Sessions/DatabaseManager.h"

//--------------------------------------------------------------------------------------------------
// Construction & destruction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initializes worker state; the SQL connection is opened lazily.
 */
Sessions::DatabaseWorker::DatabaseWorker(QObject* parent)
  : QObject(parent)
  , m_locked(false)
  , m_cancelRequested(false)
{}

/**
 * @brief Closes the SQL connection on the worker thread before destruction.
 */
Sessions::DatabaseWorker::~DatabaseWorker()
{
  closeDatabase();
}

//--------------------------------------------------------------------------------------------------
// Cancellation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the atomic cancel flag; long-running loops poll it between rows.
 */
void Sessions::DatabaseWorker::requestCancel()
{
  m_cancelRequested.store(true, std::memory_order_release);
}

//--------------------------------------------------------------------------------------------------
// File operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the SQLite file, runs schema migration, and ships caches back to the manager.
 */
void Sessions::DatabaseWorker::openDatabase(const QString& filePath)
{
  // Drop any prior connection on this worker thread
  closeDatabase();
  m_cancelRequested.store(false, std::memory_order_release);

  if (filePath.isEmpty()) {
    Q_EMIT openFailed(filePath, tr("Empty file path"));
    return;
  }

  m_filePath = filePath;
  m_connectionName =
    QStringLiteral("ss_dbmgr_%1").arg(QDateTime::currentMSecsSinceEpoch());

  m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
  m_db.setDatabaseName(filePath);

  if (!m_db.open()) {
    const QString error = m_db.lastError().text();
    const QString conn  = m_connectionName;
    m_db                = QSqlDatabase();
    if (!conn.isEmpty())
      QSqlDatabase::removeDatabase(conn);

    m_filePath.clear();
    m_connectionName.clear();
    Q_EMIT openFailed(filePath, error);
    return;
  }

  // WAL: writer + readers coexist without locks
  QSqlQuery pragma(m_db);
  pragma.exec("PRAGMA journal_mode=WAL");
  pragma.exec("PRAGMA busy_timeout=5000");

  // Schema migration + initial caches
  ensureSchemaInternal();
  refreshSessionListInternal();
  refreshTagListInternal();
  refreshLockStateInternal();

  Q_EMIT opened(m_filePath, m_sessionList, m_tagList, m_locked, m_passwordHash);
}

/**
 * @brief Closes the database, removes the connection, clears caches.
 */
void Sessions::DatabaseWorker::closeDatabase()
{
  m_cancelRequested.store(true, std::memory_order_release);

  m_sessionList.clear();
  m_tagList.clear();
  m_passwordHash.clear();
  m_locked = false;

  if (m_db.isOpen())
    m_db.close();

  const QString conn = m_connectionName;
  m_db               = QSqlDatabase();
  if (!conn.isEmpty())
    QSqlDatabase::removeDatabase(conn);

  m_filePath.clear();
  m_connectionName.clear();
  m_cancelRequested.store(false, std::memory_order_release);

  Q_EMIT closed();
}

/**
 * @brief Re-runs every internal cache fetch and ships the results back.
 */
void Sessions::DatabaseWorker::refreshAll()
{
  if (!m_db.isOpen())
    return;

  refreshSessionListInternal();
  refreshTagListInternal();
  refreshLockStateInternal();

  Q_EMIT sessionListRefreshed(m_sessionList);
  Q_EMIT tagListRefreshed(m_tagList);
  Q_EMIT lockStateChanged(m_locked, m_passwordHash);
}

//--------------------------------------------------------------------------------------------------
// Session mutations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cascades a delete across readings, raw bytes, snapshots, tags, columns, and the row.
 */
void Sessions::DatabaseWorker::deleteSession(int sessionId, quint64 token)
{
  if (!m_db.isOpen()) {
    Q_EMIT mutationFinished(token, false, tr("Database not open"));
    return;
  }

  if (!m_db.transaction()) {
    Q_EMIT mutationFinished(token, false, m_db.lastError().text());
    return;
  }

  const auto runDelete = [&](const char* sql) -> bool {
    QSqlQuery q(m_db);
    q.prepare(QString::fromLatin1(sql));
    q.bindValue(0, sessionId);
    if (!q.exec()) [[unlikely]] {
      qWarning() << "[DatabaseWorker] deleteSession" << sql << "failed:" << q.lastError().text();
      return false;
    }

    return true;
  };

  const char* statements[] = {
    "DELETE FROM readings WHERE session_id = ?",
    "DELETE FROM raw_bytes WHERE session_id = ?",
    "DELETE FROM table_snapshots WHERE session_id = ?",
    "DELETE FROM session_tags WHERE session_id = ?",
    "DELETE FROM columns WHERE session_id = ?",
    "DELETE FROM sessions WHERE session_id = ?",
  };

  for (const char* sql : statements) {
    if (!runDelete(sql)) {
      m_db.rollback();
      Q_EMIT mutationFinished(token, false, m_db.lastError().text());
      return;
    }
  }

  if (!m_db.commit()) {
    m_db.rollback();
    Q_EMIT mutationFinished(token, false, m_db.lastError().text());
    return;
  }

  refreshSessionListInternal();
  Q_EMIT sessionListRefreshed(m_sessionList);
  Q_EMIT mutationFinished(token, true, QString());
}

/**
 * @brief Persists the notes string for a session.
 */
void Sessions::DatabaseWorker::setSessionNotes(int sessionId, const QString& notes, quint64 token)
{
  if (!m_db.isOpen()) {
    Q_EMIT mutationFinished(token, false, tr("Database not open"));
    return;
  }

  QSqlQuery q(m_db);
  q.prepare("UPDATE sessions SET notes = ? WHERE session_id = ?");
  q.bindValue(0, notes);
  q.bindValue(1, sessionId);
  if (!q.exec()) {
    Q_EMIT mutationFinished(token, false, q.lastError().text());
    return;
  }

  // Refresh the row's cached notes so QML rebinds without a full list refetch
  for (auto& v : m_sessionList) {
    auto m = v.toMap();
    if (m.value("session_id").toInt() == sessionId) {
      m["notes"] = notes;
      v          = m;
      break;
    }
  }

  Q_EMIT sessionListRefreshed(m_sessionList);
  Q_EMIT notesUpdated(sessionId, notes);
  Q_EMIT mutationFinished(token, true, QString());
}

//--------------------------------------------------------------------------------------------------
// Tag mutations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Inserts a tag label, ignoring duplicates by case.
 */
void Sessions::DatabaseWorker::addTag(const QString& label, quint64 token)
{
  if (!m_db.isOpen() || label.trimmed().isEmpty()) {
    Q_EMIT mutationFinished(token, false, tr("Database not open or empty label"));
    return;
  }

  QSqlQuery q(m_db);
  q.prepare("INSERT OR IGNORE INTO tags (label) VALUES (?)");
  q.bindValue(0, label.trimmed());
  if (!q.exec()) {
    Q_EMIT mutationFinished(token, false, q.lastError().text());
    return;
  }

  refreshTagListInternal();
  Q_EMIT tagListRefreshed(m_tagList);
  Q_EMIT mutationFinished(token, true, QString());
}

/**
 * @brief Removes a tag and its session associations in a single transaction.
 */
void Sessions::DatabaseWorker::deleteTag(int tagId, quint64 token)
{
  if (!m_db.isOpen()) {
    Q_EMIT mutationFinished(token, false, tr("Database not open"));
    return;
  }

  if (!m_db.transaction()) {
    Q_EMIT mutationFinished(token, false, m_db.lastError().text());
    return;
  }

  QSqlQuery q(m_db);
  q.prepare("DELETE FROM session_tags WHERE tag_id = ?");
  q.bindValue(0, tagId);
  bool ok = q.exec();

  if (ok) {
    q.prepare("DELETE FROM tags WHERE tag_id = ?");
    q.bindValue(0, tagId);
    ok = q.exec();
  }

  if (!ok) {
    m_db.rollback();
    Q_EMIT mutationFinished(token, false, q.lastError().text());
    return;
  }

  if (!m_db.commit()) {
    m_db.rollback();
    Q_EMIT mutationFinished(token, false, m_db.lastError().text());
    return;
  }

  refreshTagListInternal();
  refreshSessionListInternal();
  Q_EMIT tagListRefreshed(m_tagList);
  Q_EMIT sessionListRefreshed(m_sessionList);
  Q_EMIT mutationFinished(token, true, QString());
}

/**
 * @brief Renames an existing tag.
 */
void Sessions::DatabaseWorker::renameTag(int tagId, const QString& newLabel, quint64 token)
{
  if (!m_db.isOpen() || newLabel.trimmed().isEmpty()) {
    Q_EMIT mutationFinished(token, false, tr("Invalid label"));
    return;
  }

  QSqlQuery q(m_db);
  q.prepare("UPDATE tags SET label = ? WHERE tag_id = ?");
  q.bindValue(0, newLabel.trimmed());
  q.bindValue(1, tagId);
  if (!q.exec()) {
    Q_EMIT mutationFinished(token, false, q.lastError().text());
    return;
  }

  refreshTagListInternal();
  refreshSessionListInternal();
  Q_EMIT tagListRefreshed(m_tagList);
  Q_EMIT sessionListRefreshed(m_sessionList);
  Q_EMIT mutationFinished(token, true, QString());
}

/**
 * @brief Associates an existing tag with a session.
 */
void Sessions::DatabaseWorker::assignTag(int sessionId, int tagId, quint64 token)
{
  if (!m_db.isOpen()) {
    Q_EMIT mutationFinished(token, false, tr("Database not open"));
    return;
  }

  QSqlQuery q(m_db);
  q.prepare("INSERT OR IGNORE INTO session_tags (session_id, tag_id) VALUES (?, ?)");
  q.bindValue(0, sessionId);
  q.bindValue(1, tagId);
  if (!q.exec()) {
    Q_EMIT mutationFinished(token, false, q.lastError().text());
    return;
  }

  refreshSessionListInternal();
  Q_EMIT sessionListRefreshed(m_sessionList);
  Q_EMIT mutationFinished(token, true, QString());
}

/**
 * @brief Removes the association between a session and a tag.
 */
void Sessions::DatabaseWorker::unassignTag(int sessionId, int tagId, quint64 token)
{
  if (!m_db.isOpen()) {
    Q_EMIT mutationFinished(token, false, tr("Database not open"));
    return;
  }

  QSqlQuery q(m_db);
  q.prepare("DELETE FROM session_tags WHERE session_id = ? AND tag_id = ?");
  q.bindValue(0, sessionId);
  q.bindValue(1, tagId);
  if (!q.exec()) {
    Q_EMIT mutationFinished(token, false, q.lastError().text());
    return;
  }

  refreshSessionListInternal();
  Q_EMIT sessionListRefreshed(m_sessionList);
  Q_EMIT mutationFinished(token, true, QString());
}

//--------------------------------------------------------------------------------------------------
// Lock state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes the lock password hash into project_metadata, or removes it.
 */
void Sessions::DatabaseWorker::persistLock(const QString& passwordHash, quint64 token)
{
  if (!m_db.isOpen()) {
    Q_EMIT mutationFinished(token, false, tr("Database not open"));
    return;
  }

  QSqlQuery q(m_db);
  if (passwordHash.isEmpty()) {
    q.prepare("DELETE FROM project_metadata WHERE key = 'lock_password_hash'");
    if (!q.exec()) {
      Q_EMIT mutationFinished(token, false, q.lastError().text());
      return;
    }
  } else {
    q.prepare("INSERT OR REPLACE INTO project_metadata (key, value) "
              "VALUES ('lock_password_hash', ?)");
    q.bindValue(0, passwordHash);
    if (!q.exec()) {
      Q_EMIT mutationFinished(token, false, q.lastError().text());
      return;
    }
  }

  m_passwordHash = passwordHash;
  m_locked       = !passwordHash.isEmpty();

  Q_EMIT lockStateChanged(m_locked, m_passwordHash);
  Q_EMIT mutationFinished(token, true, QString());
}

//--------------------------------------------------------------------------------------------------
// Project metadata
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores the live project JSON snapshot in project_metadata.
 */
void Sessions::DatabaseWorker::storeProjectMetadata(const QString& projectJson,
                                                    const QString& projectTitle,
                                                    quint64 token)
{
  if (!m_db.isOpen()) {
    Q_EMIT mutationFinished(token, false, tr("Database not open"));
    return;
  }

  if (!m_db.transaction()) {
    Q_EMIT mutationFinished(token, false, m_db.lastError().text());
    return;
  }

  const auto now = QDateTime::currentDateTime().toString(Qt::ISODate);

  QSqlQuery q(m_db);
  bool ok = true;

  q.prepare("INSERT OR REPLACE INTO project_metadata (key, value) VALUES ('project_json', ?)");
  q.bindValue(0, projectJson);
  ok = ok && q.exec();

  q.prepare("INSERT OR REPLACE INTO project_metadata (key, value) VALUES ('project_title', ?)");
  q.bindValue(0, projectTitle);
  ok = ok && q.exec();

  q.prepare(
    "INSERT OR REPLACE INTO project_metadata (key, value) VALUES ('last_modified_at', ?)");
  q.bindValue(0, now);
  ok = ok && q.exec();

  q.prepare("INSERT OR IGNORE INTO project_metadata (key, value) VALUES ('created_at', ?)");
  q.bindValue(0, now);
  ok = ok && q.exec();

  if (!ok) {
    m_db.rollback();
    Q_EMIT mutationFinished(token, false, q.lastError().text());
    return;
  }

  if (!m_db.commit()) {
    m_db.rollback();
    Q_EMIT mutationFinished(token, false, m_db.lastError().text());
    return;
  }

  Q_EMIT mutationFinished(token, true, QString());
}

/**
 * @brief Reads the global project JSON snapshot back to the manager.
 */
void Sessions::DatabaseWorker::fetchGlobalProjectJson()
{
  if (!m_db.isOpen()) {
    Q_EMIT globalProjectJsonReady(QString());
    return;
  }

  QSqlQuery q(m_db);
  q.prepare("SELECT value FROM project_metadata WHERE key = 'project_json'");
  if (q.exec() && q.next())
    Q_EMIT globalProjectJsonReady(q.value(0).toString());
  else
    Q_EMIT globalProjectJsonReady(QString());
}

//--------------------------------------------------------------------------------------------------
// CSV export (streaming on the worker thread)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Streams a session's readings into a CSV file row-by-row, emitting progress.
 */
void Sessions::DatabaseWorker::runCsvExport(int sessionId, const QString& outputPath)
{
  m_cancelRequested.store(false, std::memory_order_release);

  if (!m_db.isOpen()) {
    Q_EMIT csvExportFinished(outputPath, false, tr("Database not open"));
    return;
  }

  // Load column order
  QSqlQuery colQ(m_db);
  colQ.prepare("SELECT unique_id, group_title, title, units FROM columns "
               "WHERE session_id = ? ORDER BY column_id ASC");
  colQ.bindValue(0, sessionId);
  if (!colQ.exec()) {
    Q_EMIT csvExportFinished(outputPath, false, colQ.lastError().text());
    return;
  }

  // Build header from column metadata
  std::vector<int> uniqueIds;
  QStringList headerCells;
  headerCells.append("Timestamp (s)");
  while (colQ.next()) {
    uniqueIds.push_back(colQ.value(0).toInt());
    const auto group = colQ.value(1).toString();
    const auto title = colQ.value(2).toString();
    const auto units = colQ.value(3).toString();
    auto label       = group + "/" + title;
    if (!units.isEmpty())
      label += " (" + units + ")";

    headerCells.append(label);
  }

  // Approx total reading count for progress reporting
  qint64 totalRows = 0;
  {
    QSqlQuery cnt(m_db);
    cnt.prepare("SELECT COUNT(*) FROM readings WHERE session_id = ?");
    cnt.bindValue(0, sessionId);
    if (cnt.exec() && cnt.next())
      totalRows = cnt.value(0).toLongLong();
  }

  // uid → column index
  QMap<int, int> uidToCol;
  for (int i = 0; i < static_cast<int>(uniqueIds.size()); ++i)
    uidToCol.insert(uniqueIds[static_cast<size_t>(i)], i);

  QSqlQuery readQ(m_db);
  readQ.setForwardOnly(true);
  readQ.prepare(
    "SELECT timestamp_ns, unique_id, final_numeric_value, final_string_value, is_numeric "
    "FROM readings WHERE session_id = ? ORDER BY timestamp_ns, reading_id");
  readQ.bindValue(0, sessionId);
  if (!readQ.exec()) {
    Q_EMIT csvExportFinished(outputPath, false, readQ.lastError().text());
    return;
  }

  QFile file(outputPath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    Q_EMIT csvExportFinished(outputPath, false, file.errorString());
    return;
  }

  QTextStream out(&file);
  out << headerCells.join(',') << '\n';

  qint64 currentTs = -1;
  QStringList row;
  const int colCount     = static_cast<int>(uniqueIds.size());
  qint64 processed       = 0;
  qint64 nextProgressTick = totalRows > 0 ? (totalRows / 100 + 1) : 0;

  auto flushRow = [&]() {
    if (currentTs < 0 || row.isEmpty())
      return;

    out << QString::number(currentTs / 1e9, 'f', 9);
    for (const auto& cell : std::as_const(row))
      out << ',' << cell;

    out << '\n';
  };

  auto startRow = [&](qint64 ts) {
    currentTs = ts;
    row.clear();
    row.reserve(colCount);
    for (int i = 0; i < colCount; ++i)
      row.append(QString());
  };

  while (readQ.next()) {
    if (m_cancelRequested.load(std::memory_order_acquire)) {
      file.close();
      file.remove();
      Q_EMIT csvExportFinished(outputPath, false, tr("Cancelled"));
      return;
    }

    const qint64 ts  = readQ.value(0).toLongLong();
    const int uid    = readQ.value(1).toInt();
    const auto colIt = uidToCol.constFind(uid);
    if (colIt == uidToCol.constEnd())
      continue;

    if (ts != currentTs) {
      flushRow();
      startRow(ts);
    }

    const bool isNumeric = readQ.value(4).toInt() != 0;
    const int col        = colIt.value();
    if (isNumeric)
      row[col] = QString::number(readQ.value(2).toDouble(), 'g', 17);
    else
      row[col] = readQ.value(3).toString();

    ++processed;
    if (totalRows > 0 && (processed % nextProgressTick == 0)) {
      const double pct = static_cast<double>(processed) / static_cast<double>(totalRows);
      Q_EMIT csvExportProgress(std::clamp(pct, 0.0, 1.0));
    }
  }

  flushRow();
  file.close();

  Q_EMIT csvExportProgress(1.0);
  Q_EMIT csvExportFinished(outputPath, true, QString());
}

//--------------------------------------------------------------------------------------------------
// Report data load (SQL only — rendering stays on main thread)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the report data + chart series bundle, ships it via shared_ptr.
 */
void Sessions::DatabaseWorker::runReportDataLoad(int sessionId, bool includeCharts,
                                                  int chartMaxSamples)
{
  auto payload       = std::make_shared<ReportPayload>();
  payload->sessionId = sessionId;
  payload->ok        = false;

  if (!m_db.isOpen()) {
    payload->error = tr("Database not open");
    Q_EMIT reportDataReady(payload);
    return;
  }

  payload->data = ReportData::buildFromSession(m_db, sessionId);
  if (!payload->data.valid) {
    payload->error = tr("Could not load session data");
    Q_EMIT reportDataReady(payload);
    return;
  }

  if (includeCharts)
    payload->series = loadChartSeries(m_db, sessionId, chartMaxSamples);

  payload->ok = true;
  Q_EMIT reportDataReady(payload);
}

//--------------------------------------------------------------------------------------------------
// Internal cache fetchers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reloads the session list cache from the open database.
 */
void Sessions::DatabaseWorker::refreshSessionListInternal()
{
  m_sessionList.clear();
  if (!m_db.isOpen())
    return;

  QSqlQuery q(m_db);
  q.prepare("SELECT s.session_id, s.project_title, s.started_at, s.ended_at, s.notes, "
            "       (SELECT COUNT(DISTINCT timestamp_ns) FROM readings "
            "        WHERE session_id = s.session_id) "
            "FROM sessions s ORDER BY s.started_at DESC");
  if (!q.exec())
    return;

  while (q.next()) {
    if (m_cancelRequested.load(std::memory_order_acquire))
      return;

    QVariantMap row;
    const int sessionId    = q.value(0).toInt();
    row["session_id"]      = sessionId;
    row["project_title"]   = q.value(1).toString();
    row["started_at_iso"]  = q.value(2).toString();
    row["ended_at_iso"]    = q.value(3).toString();
    row["started_at"]      = formatDateForDisplay(q.value(2).toString());
    row["ended_at"]        = formatDateForDisplay(q.value(3).toString());
    row["notes"]           = q.value(4).toString();
    row["frame_count"]     = q.value(5).toInt();

    const auto tags = tagsForSession(sessionId);
    QStringList labels;
    labels.reserve(tags.size());
    for (const auto& t : tags)
      labels.append(t.toMap().value("label").toString());

    row["tags"]       = tags;
    row["tag_labels"] = labels.join(", ");

    m_sessionList.append(row);
  }
}

/**
 * @brief Reloads the global tag list cache.
 */
void Sessions::DatabaseWorker::refreshTagListInternal()
{
  m_tagList.clear();
  if (!m_db.isOpen())
    return;

  QSqlQuery q(m_db);
  q.prepare("SELECT tag_id, label FROM tags ORDER BY label COLLATE NOCASE");
  if (!q.exec())
    return;

  while (q.next()) {
    QVariantMap tag;
    tag["tag_id"] = q.value(0).toInt();
    tag["label"]  = q.value(1).toString();
    m_tagList.append(tag);
  }
}

/**
 * @brief Reloads the lock password hash from project_metadata.
 */
void Sessions::DatabaseWorker::refreshLockStateInternal()
{
  m_passwordHash.clear();
  m_locked = false;

  if (!m_db.isOpen())
    return;

  QSqlQuery q(m_db);
  q.prepare("SELECT value FROM project_metadata WHERE key = 'lock_password_hash'");
  if (q.exec() && q.next())
    m_passwordHash = q.value(0).toString();

  m_locked = !m_passwordHash.isEmpty();
}

/**
 * @brief Runs the shared CREATE TABLE IF NOT EXISTS block.
 */
void Sessions::DatabaseWorker::ensureSchemaInternal()
{
  if (!m_db.isOpen())
    return;

  QSqlQuery q(m_db);
  Sessions::DatabaseManager::createSchema(q);
}

/**
 * @brief Returns the tags assigned to a session as a QVariantList of {tag_id, label} maps.
 */
QVariantList Sessions::DatabaseWorker::tagsForSession(int sessionId)
{
  QVariantList result;
  if (!m_db.isOpen())
    return result;

  QSqlQuery q(m_db);
  q.prepare("SELECT t.tag_id, t.label FROM tags t "
            "JOIN session_tags st ON st.tag_id = t.tag_id "
            "WHERE st.session_id = ? ORDER BY t.label");
  q.bindValue(0, sessionId);
  if (!q.exec())
    return result;

  while (q.next()) {
    QVariantMap tag;
    tag["tag_id"] = q.value(0).toInt();
    tag["label"]  = q.value(1).toString();
    result.append(tag);
  }

  return result;
}

/**
 * @brief Formats an ISO 8601 date string into a user-friendly display string.
 */
QString Sessions::DatabaseWorker::formatDateForDisplay(const QString& isoDate)
{
  if (isoDate.isEmpty())
    return {};

  const auto dt = QDateTime::fromString(isoDate, Qt::ISODate);
  if (!dt.isValid())
    return isoDate;

  return dt.toString(QStringLiteral("MMM d, yyyy — h:mm AP"));
}

#endif  // BUILD_COMMERCIAL

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

#  include "Sessions/PlayerLoaderWorker.h"

#  include <QDateTime>
#  include <QObject>
#  include <QSqlDatabase>
#  include <QSqlError>
#  include <QSqlQuery>

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initializes the cancellation flag.
 */
Sessions::PlayerLoaderWorker::PlayerLoaderWorker(QObject* parent)
  : QObject(parent)
  , m_cancelRequested(false)
{}

/**
 * @brief Trivial destructor — no owned resources outside per-call scope.
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
 * @brief Opens the file, fetches column order + project JSON + timestamp index, ships it back.
 * @param filePath  Absolute path to the .db file.
 * @param sessionId Session to load, or -1 to pick the most recent.
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

  // Open with a unique connection name on this worker thread
  const QString connName =
    QStringLiteral("ss_player_loader_%1").arg(QDateTime::currentMSecsSinceEpoch());

  // Scope the connection so we always remove it before returning
  {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
    db.setDatabaseName(filePath);

    if (!db.open()) {
      payload->error = db.lastError().text();
      db = QSqlDatabase();
      QSqlDatabase::removeDatabase(connName);
      Q_EMIT loaded(payload);
      return;
    }

    QSqlQuery pragma(db);
    pragma.exec("PRAGMA journal_mode=WAL");
    pragma.exec("PRAGMA busy_timeout=5000");

    // Pick the latest session if the caller did not specify one
    if (sessionId < 0) {
      QSqlQuery q(db);
      q.prepare("SELECT session_id FROM sessions ORDER BY started_at DESC LIMIT 1");
      if (!q.exec() || !q.next()) {
        payload->error = tr("This file does not contain any recording sessions.");
        db.close();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connName);
        Q_EMIT loaded(payload);
        return;
      }

      sessionId          = q.value(0).toInt();
      payload->sessionId = sessionId;
    }

    if (m_cancelRequested.load(std::memory_order_acquire)) {
      db.close();
      db = QSqlDatabase();
      QSqlDatabase::removeDatabase(connName);
      payload->error = tr("Cancelled");
      Q_EMIT loaded(payload);
      return;
    }

    // Per-session embedded project, falling back to the global metadata row
    {
      QSqlQuery q(db);
      q.prepare("SELECT project_json FROM sessions WHERE session_id = ?");
      q.bindValue(0, sessionId);
      if (q.exec() && q.next())
        payload->projectJson = q.value(0).toString();
    }

    if (payload->projectJson.isEmpty()) {
      QSqlQuery q(db);
      q.prepare("SELECT value FROM project_metadata WHERE key = 'project_json'");
      if (q.exec() && q.next())
        payload->projectJson = q.value(0).toString();
    }

    // Column order — must match the live FrameBuilder
    {
      QSqlQuery q(db);
      q.setForwardOnly(true);
      q.prepare("SELECT unique_id FROM columns WHERE session_id = ? ORDER BY column_id ASC");
      q.bindValue(0, sessionId);
      if (!q.exec()) {
        payload->error = q.lastError().text();
        db.close();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connName);
        Q_EMIT loaded(payload);
        return;
      }

      while (q.next())
        payload->columnUniqueIds.push_back(q.value(0).toInt());
    }

    if (payload->columnUniqueIds.empty()) {
      payload->error = tr("The selected session is missing its column definitions.");
      db.close();
      db = QSqlDatabase();
      QSqlDatabase::removeDatabase(connName);
      Q_EMIT loaded(payload);
      return;
    }

    if (m_cancelRequested.load(std::memory_order_acquire)) {
      db.close();
      db = QSqlDatabase();
      QSqlDatabase::removeDatabase(connName);
      payload->error = tr("Cancelled");
      Q_EMIT loaded(payload);
      return;
    }

    // Timestamp index — the heaviest single query on big sessions
    {
      QSqlQuery q(db);
      q.setForwardOnly(true);
      q.prepare("SELECT DISTINCT timestamp_ns FROM readings "
                "WHERE session_id = ? ORDER BY timestamp_ns ASC");
      q.bindValue(0, sessionId);
      if (!q.exec()) {
        payload->error = q.lastError().text();
        db.close();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connName);
        Q_EMIT loaded(payload);
        return;
      }

      // Periodic cancel checks for huge result sets
      qint64 row = 0;
      while (q.next()) {
        if ((row & 0xFFFF) == 0
            && m_cancelRequested.load(std::memory_order_acquire)) {
          db.close();
          db = QSqlDatabase();
          QSqlDatabase::removeDatabase(connName);
          payload->error = tr("Cancelled");
          Q_EMIT loaded(payload);
          return;
        }

        payload->timestampsNs.push_back(q.value(0).toLongLong());
        ++row;
      }
    }

    if (payload->timestampsNs.empty()) {
      payload->error = tr("The selected session does not contain any frames.");
      db.close();
      db = QSqlDatabase();
      QSqlDatabase::removeDatabase(connName);
      Q_EMIT loaded(payload);
      return;
    }

    db.close();
  }

  QSqlDatabase::removeDatabase(connName);

  payload->ok = true;
  Q_EMIT loaded(payload);
}

#endif  // BUILD_COMMERCIAL

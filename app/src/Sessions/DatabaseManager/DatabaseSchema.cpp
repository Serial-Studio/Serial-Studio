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

#  include <QApplication>
#  include <QCoreApplication>
#  include <QDateTime>
#  include <QDir>
#  include <QFile>
#  include <QFileDialog>
#  include <QFileInfo>
#  include <QGuiApplication>
#  include <QInputDialog>
#  include <QJsonDocument>
#  include <QJsonObject>
#  include <QJsonParseError>
#  include <QProcess>
#  include <QSqlError>
#  include <QSqlQuery>
#  include <QThread>
#  include <QTimer>

#  include "AppState.h"
#  include "DataModel/ProjectModel.h"
#  include "Misc/PasswordHash.h"
#  include "Misc/Utilities.h"
#  include "Misc/WorkspaceManager.h"
#  include "SerialStudio.h"
#  include "Sessions/DatabaseManager.h"
#  include "Sessions/Export.h"
#  include "Sessions/HtmlReport.h"
#  include "Sessions/Player.h"
#  include "Sessions/ReportData.h"
#  include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Schema (shared with Sessions::Export at session creation time)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates or upgrades the session-log schema on the open database.
 */
void Sessions::DatabaseManager::createSchema(QSqlQuery& q)
{
  createSchemaSessionTables(q);
  migrateColumnsTable(q);
  migrateSessionsTable(q);
  createSchemaSampleTables(q);
  createSchemaStreamTables(q);
  createSchemaBlockTable(q);
  createSchemaTagTables(q);
  createSchemaProjectMetadata(q);
  createSchemaVerifications(q);
  q.exec(QStringLiteral("PRAGMA user_version = %1").arg(kUserVersion));
}

/**
 * @brief Creates the sessions header and columns metadata tables.
 */
void Sessions::DatabaseManager::createSchemaSessionTables(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS sessions ("
         "  session_id    INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  project_title TEXT NOT NULL,"
         "  started_at    TEXT NOT NULL,"
         "  ended_at      TEXT,"
         "  project_json  TEXT,"
         "  notes         TEXT"
         ")");

  q.exec("CREATE TABLE IF NOT EXISTS columns ("
         "  column_id    INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id   INTEGER NOT NULL REFERENCES sessions,"
         "  unique_id    INTEGER NOT NULL,"
         "  source_id    INTEGER NOT NULL DEFAULT 0,"
         "  source_title TEXT NOT NULL DEFAULT '',"
         "  group_title  TEXT NOT NULL,"
         "  title        TEXT NOT NULL,"
         "  units        TEXT,"
         "  widget       TEXT,"
         "  is_virtual   INTEGER NOT NULL DEFAULT 0"
         ")");
}

/**
 * @brief Adds source_id / source_title to legacy columns tables in older databases.
 */
void Sessions::DatabaseManager::migrateColumnsTable(QSqlQuery& q)
{
  auto columnExists = [&q](const QString& column) {
    if (!q.exec(QStringLiteral("PRAGMA table_info(\"columns\")"))) {
      qWarning() << "[Sessions] PRAGMA table_info failed:" << q.lastError().text();
      return false;
    }
    while (q.next())
      if (q.value(1).toString().compare(column, Qt::CaseInsensitive) == 0)
        return true;

    return false;
  };

  if (!columnExists(QStringLiteral("source_id"))) {
    if (!q.exec("ALTER TABLE \"columns\" ADD COLUMN source_id INTEGER NOT NULL DEFAULT 0"))
      qWarning() << "[Sessions] ALTER add source_id failed:" << q.lastError().text();
  }

  if (!columnExists(QStringLiteral("source_title"))) {
    if (!q.exec("ALTER TABLE \"columns\" ADD COLUMN source_title TEXT NOT NULL DEFAULT ''"))
      qWarning() << "[Sessions] ALTER add source_title failed:" << q.lastError().text();
  }
}

/**
 * @brief Adds the spec-0044 fingerprint/classification columns and the spec-0062 view-state
 *        bundle column to legacy sessions tables (nullable, so old archives keep reading).
 */
void Sessions::DatabaseManager::migrateSessionsTable(QSqlQuery& q)
{
  auto columnExists = [&q](const QString& column) {
    if (!q.exec(QStringLiteral("PRAGMA table_info(\"sessions\")"))) {
      qWarning() << "[Sessions] PRAGMA table_info failed:" << q.lastError().text();
      return false;
    }
    while (q.next())
      if (q.value(1).toString().compare(column, Qt::CaseInsensitive) == 0)
        return true;

    return false;
  };

  static constexpr struct {
    const char* name;
    const char* type;
  } kColumns[] = {
    {     "raw_sha256",    "TEXT"},
    {"readings_sha256",    "TEXT"},
    {    "app_version",    "TEXT"},
    { "capture_format", "INTEGER"},
    {    "repro_class",    "TEXT"},
    { "frames_dropped", "INTEGER"},
    { "overflow_bytes", "INTEGER"},
    {  "stream_sha256",    "TEXT"},
    {     "view_state",    "TEXT"},
  };

  for (const auto& col : kColumns) {
    if (columnExists(QLatin1String(col.name)))
      continue;

    const auto sql = QStringLiteral("ALTER TABLE \"sessions\" ADD COLUMN %1 %2")
                       .arg(QLatin1String(col.name), QLatin1String(col.type));
    if (!q.exec(sql))
      qWarning() << "[Sessions] ALTER add" << col.name << "failed:" << q.lastError().text();
  }
}

/**
 * @brief Creates the per-sample tables (readings, raw_bytes, table_snapshots) and indexes.
 */
void Sessions::DatabaseManager::createSchemaSampleTables(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS readings ("
         "  reading_id          INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id          INTEGER NOT NULL,"
         "  timestamp_ns        INTEGER NOT NULL,"
         "  unique_id           INTEGER NOT NULL,"
         "  raw_numeric_value   REAL,"
         "  raw_string_value    TEXT,"
         "  final_numeric_value REAL,"
         "  final_string_value  TEXT,"
         "  is_numeric          INTEGER NOT NULL DEFAULT 1"
         ")");
  q.exec("CREATE INDEX IF NOT EXISTS idx_readings_session_uid_ts "
         "ON readings (session_id, unique_id, timestamp_ns)");
  q.exec("CREATE INDEX IF NOT EXISTS idx_readings_session_ts "
         "ON readings (session_id, timestamp_ns)");

  q.exec("CREATE TABLE IF NOT EXISTS raw_bytes ("
         "  raw_id       INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id   INTEGER NOT NULL,"
         "  timestamp_ns INTEGER NOT NULL,"
         "  device_id    INTEGER NOT NULL DEFAULT 0,"
         "  data         BLOB NOT NULL"
         ")");
  q.exec("CREATE INDEX IF NOT EXISTS idx_raw_bytes_session_ts "
         "ON raw_bytes (session_id, timestamp_ns)");

  q.exec("CREATE TABLE IF NOT EXISTS table_snapshots ("
         "  snapshot_id   INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id    INTEGER NOT NULL,"
         "  timestamp_ns  INTEGER NOT NULL,"
         "  table_name    TEXT NOT NULL,"
         "  register_name TEXT NOT NULL,"
         "  numeric_value REAL,"
         "  string_value  TEXT"
         ")");
  q.exec("CREATE INDEX IF NOT EXISTS idx_snapshots_session_ts "
         "ON table_snapshots (session_id, timestamp_ns, table_name)");
}

/**
 * @brief Creates the full-rate stream-block table (spec 0054). One row per acquisition block
 *        per dataset: `samples` is `frames` IEEE-754 float64 values written little-endian, and
 *        `t0_ns` + `dt_ns` date each of them as t0 + i * dt, so the source keeps owning time.
 *        Additive only -- a v1 database gains the empty table and replays unchanged.
 */
void Sessions::DatabaseManager::createSchemaStreamTables(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS stream_blocks ("
         "  stream_block_id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id      INTEGER NOT NULL,"
         "  source_id       INTEGER NOT NULL,"
         "  unique_id       INTEGER NOT NULL,"
         "  block_number    INTEGER NOT NULL,"
         "  t0_ns           INTEGER NOT NULL,"
         "  dt_ns           INTEGER NOT NULL,"
         "  frames          INTEGER NOT NULL,"
         "  samples         BLOB NOT NULL"
         ")");
  q.exec("CREATE INDEX IF NOT EXISTS idx_stream_blocks_session_uid_t0 "
         "ON stream_blocks (session_id, unique_id, t0_ns)");
}

/**
 * @brief Creates the unified block table (spec 0055 R7): one row per dataset per published block,
 *        for BOTH lanes. Little-endian float64 values plus their pre-transform twin, length-
 *        prefixed UTF-8 texts, and explicit per-sample times when `dt_ns` is 0. Additive -- v1/v2
 *        databases keep `readings` and `stream_blocks` and still replay.
 */
void Sessions::DatabaseManager::createSchemaBlockTable(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS blocks ("
         "  block_id     INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id   INTEGER NOT NULL,"
         "  source_id    INTEGER NOT NULL,"
         "  unique_id    INTEGER NOT NULL,"
         "  block_number INTEGER NOT NULL,"
         "  t0_ns        INTEGER NOT NULL,"
         "  t_end_ns     INTEGER NOT NULL,"
         "  dt_ns        INTEGER NOT NULL,"
         "  frames       INTEGER NOT NULL,"
         "  is_numeric   INTEGER NOT NULL DEFAULT 1,"
         "  min_value    REAL,"
         "  max_value    REAL,"
         "  sum_value    REAL,"
         "  finite_count INTEGER NOT NULL DEFAULT 0,"
         "  values_blob  BLOB NOT NULL,"
         "  raw_values   BLOB,"
         "  texts        BLOB,"
         "  raw_texts    BLOB,"
         "  times        BLOB"
         ")");
  q.exec("CREATE INDEX IF NOT EXISTS idx_blocks_session_uid_t0 "
         "ON blocks (session_id, unique_id, t0_ns)");
  q.exec("CREATE INDEX IF NOT EXISTS idx_blocks_session_t0 "
         "ON blocks (session_id, t0_ns, t_end_ns)");
}

/**
 * @brief Creates the tags catalog and the session -> tag join table.
 */
void Sessions::DatabaseManager::createSchemaTagTables(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS tags ("
         "  tag_id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  label  TEXT NOT NULL UNIQUE COLLATE NOCASE"
         ")");

  q.exec("CREATE TABLE IF NOT EXISTS session_tags ("
         "  session_id INTEGER NOT NULL,"
         "  tag_id     INTEGER NOT NULL,"
         "  PRIMARY KEY (session_id, tag_id)"
         ") WITHOUT ROWID");
}

/**
 * @brief Creates the project_metadata key/value store.
 */
void Sessions::DatabaseManager::createSchemaProjectMetadata(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS project_metadata ("
         "  key   TEXT PRIMARY KEY,"
         "  value TEXT NOT NULL"
         ") WITHOUT ROWID");
}

/**
 * @brief Creates the append-only verification-record table (spec 0044).
 */
void Sessions::DatabaseManager::createSchemaVerifications(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS verifications ("
         "  verification_id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id      INTEGER NOT NULL REFERENCES sessions,"
         "  verified_at     TEXT NOT NULL,"
         "  app_version     TEXT NOT NULL,"
         "  verdict         TEXT NOT NULL,"
         "  detail_json     TEXT"
         ")");
  q.exec("CREATE INDEX IF NOT EXISTS idx_verifications_session "
         "ON verifications (session_id, verification_id)");
}

#endif

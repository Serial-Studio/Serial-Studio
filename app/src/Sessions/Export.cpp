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

#  include "Sessions/Export.h"

#  include <bit>
#  include <cmath>
#  include <cstring>
#  include <QDateTime>
#  include <QDir>
#  include <QJsonArray>
#  include <QJsonDocument>
#  include <QJsonObject>
#  include <QSqlError>
#  include <QtEndian>

#  include "AppInfo.h"
#  include "AppState.h"
#  include "CSV/Player.h"
#  include "DataModel/FrameBuilder.h"
#  include "DataModel/ProjectModel.h"
#  include "DataModel/Scripting/ControlScript.h"
#  include "DataModel/Scripting/FrameParser.h"
#  include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#  include "IO/ConnectionManager.h"
#  include "Licensing/CommercialToken.h"
#  include "Licensing/LemonSqueezy.h"
#  include "MDF4/Player.h"
#  include "Misc/TimerEvents.h"
#  include "Misc/WorkspaceManager.h"
#  include "Sessions/DatabaseManager.h"
#  include "Sessions/StreamBlockCodec.h"
#  include "SSAssert.h"
#  include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// View-state pushes coalesce on the autosave debounce (spec 0062 R3)
static constexpr int kViewStateDebounceMs = 1500;

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

//--------------------------------------------------------------------------------------------------
// ExportWorker implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the SQLite export worker with both frame and raw bytes queues.
 */
Sessions::ExportWorker::ExportWorker(
  moodycamel::ReaderWriterQueue<DataModel::DataBlockPtr>* blockQueue,
  std::atomic<bool>* enabled,
  std::atomic<size_t>* queueSize,
  moodycamel::ReaderWriterQueue<TimestampedRawBytes>* rawQueue,
  moodycamel::ReaderWriterQueue<TableSnapshotEntry>* snapshotQueue,
  std::atomic<int>* operationMode,
  QMutex* projectSnapshotMutex,
  const QByteArray* projectSnapshot,
  const QByteArray* viewStateSnapshot,
  const std::atomic<bool>* controlScriptSeen,
  const std::atomic<quint64>* linkDroppedFrames,
  const std::atomic<quint64>* linkOverflowBytes)
  : DataModel::FrameConsumerWorker<DataModel::DataBlockPtr>(blockQueue, enabled, queueSize)
  , m_dbOpen(false)
  , m_sessionId(-1)
  , m_lastRawBytesNs(-1)
  , m_rawHash(QCryptographicHash::Sha256)
  , m_blocksHash(QCryptographicHash::Sha256)
  , m_rawQueue(rawQueue)
  , m_snapshotQueue(snapshotQueue)
  , m_operationMode(operationMode)
  , m_projectSnapshotMutex(projectSnapshotMutex)
  , m_projectSnapshot(projectSnapshot)
  , m_viewStateSnapshot(viewStateSnapshot)
  , m_controlScriptSeen(controlScriptSeen)
  , m_linkDroppedFrames(linkDroppedFrames)
  , m_linkOverflowBytes(linkOverflowBytes)
{
  SS_ASSERT_LOG(rawQueue != nullptr);
  SS_ASSERT_LOG(snapshotQueue != nullptr);
  SS_ASSERT_LOG(blockQueue != nullptr);
  SS_ASSERT_LOG(operationMode != nullptr);
  SS_ASSERT_LOG(projectSnapshotMutex != nullptr);
  SS_ASSERT_LOG(projectSnapshot != nullptr);
  SS_ASSERT_LOG(viewStateSnapshot != nullptr);
  SS_ASSERT_LOG(controlScriptSeen != nullptr);
  SS_ASSERT_LOG(linkDroppedFrames != nullptr);
  SS_ASSERT_LOG(linkOverflowBytes != nullptr);
}

/**
 * @brief Default destructor.
 */
Sessions::ExportWorker::~ExportWorker() = default;

/**
 * @brief Closes the database and finalizes the current session.
 */
void Sessions::ExportWorker::closeResources()
{
  if (!m_dbOpen)
    return;

  finalizeSession();

  TableSnapshotEntry staleSnapshot;
  while (m_snapshotQueue->try_dequeue(staleSnapshot)) {
  }

  m_blockQuery.reset();
  m_rawBytesQuery.reset();
  m_tableSnapshotQuery.reset();

  QString connName;
  if (m_db) {
    connName = m_db->connectionName();
    QSqlQuery checkpoint(*m_db);
    checkpoint.exec("PRAGMA wal_checkpoint(RESTART)");
    m_db->close();
  }

  m_db.reset();
  if (!connName.isEmpty())
    QSqlDatabase::removeDatabase(connName);

  m_dbOpen         = false;
  m_sessionId      = -1;
  m_lastRawBytesNs = -1;
  m_schema         = DataModel::ExportSchema{};
  m_rawHash.reset();
  m_blocksHash.reset();
  resetMonotonicClock();
}

/**
 * @brief Returns whether the database file is open.
 */
bool Sessions::ExportWorker::isResourceOpen() const
{
  return m_dbOpen;
}

/**
 * @brief Override processData to drain both frame and raw bytes queues.
 */
void Sessions::ExportWorker::processData()
{
  DataModel::FrameConsumerWorker<DataModel::DataBlockPtr>::processData();

  if (!consumerEnabled())
    return;

  if (!m_dbOpen
      && m_operationMode->load(std::memory_order_relaxed)
           == static_cast<int>(SerialStudio::ConsoleOnly)) {
    if (const auto* head = m_rawQueue->peek()) {
      DataModel::Frame emptyFrame;
      emptyFrame.title = QStringLiteral("ConsoleOnly");
      createDatabase(emptyFrame);
      if (m_dbOpen) {
        m_steadyBaseline = head->data->timestamp;
        Q_EMIT resourceOpenChanged();
      }
    }
  }

  if (m_dbOpen) {
    writeRawBytes();
    writeTableSnapshots();
  }
}

/**
 * @brief Writes one published block: one row per dataset column, values and their pre-transform
 *        twin as canonical little-endian blobs. A uniform grid stores t0 + dt and no time array;
 *        an irregular block stores dt = 0 and explicit per-sample offsets, so the two lanes share
 *        one layout and the source keeps owning time either way.
 */
void Sessions::ExportWorker::writeBlocks(const DataModel::DataBlockPtr& block)
{
  const bool uniform = DataModel::uniform_grid(*block);
  const qint64 t0Ns =
    std::chrono::duration_cast<std::chrono::nanoseconds>(block->t0 - m_steadyBaseline).count();
  const qint64 dtNs = uniform ? block->dt.count() : 0;

  for (const auto& column : block->columns)
    insertBlockRow(*block, column, t0Ns, dtNs);
}

/**
 * @brief Binds and executes one block row, feeding the fingerprint only after a successful insert
 *        so the digest never covers rows the archive does not contain.
 */
void Sessions::ExportWorker::insertBlockRow(const DataModel::DataBlock& block,
                                            const DataModel::BlockColumn& column,
                                            qint64 t0Ns,
                                            qint64 dtNs)
{
  if (!m_blockQuery) [[unlikely]]
    return;

  const auto used = static_cast<std::size_t>(block.samples);

  const QByteArray valueBlob = packStreamSamples({column.values.data(), used});

  QByteArray rawBlob;
  if (column.hasRaw)
    rawBlob = packStreamSamples({column.rawValues.data(), used});

  QByteArray textBlob;
  QByteArray rawTextBlob;
  bool numeric = true;
  if (column.hasText) {
    textBlob = packStreamText({column.text.data(), used});

    for (qsizetype i = 0; i < block.samples && numeric; ++i)
      numeric = DataModel::sample_is_numeric(column, i);

    if (column.hasRaw)
      rawTextBlob = packStreamText({column.rawText.data(), used});
  }

  QByteArray timesBlob;
  if (dtNs == 0)
    timesBlob = packStreamTimes({block.times.data(), used});

  m_blockQuery->bindValue(0, m_sessionId);
  m_blockQuery->bindValue(1, block.sourceId);
  m_blockQuery->bindValue(2, column.uniqueId);
  m_blockQuery->bindValue(3, static_cast<qint64>(block.blockNumber));
  const qint64 lastOffset =
    dtNs != 0
      ? dtNs * (block.samples - 1)
      : (block.times.empty() ? 0 : block.times[static_cast<std::size_t>(block.samples - 1)]);

  m_blockQuery->bindValue(4, t0Ns);
  m_blockQuery->bindValue(5, t0Ns + lastOffset);
  m_blockQuery->bindValue(6, dtNs);
  m_blockQuery->bindValue(7, static_cast<qint64>(block.samples));
  double minValue    = 0.0;
  double maxValue    = 0.0;
  double sumValue    = 0.0;
  qint64 finiteCount = 0;
  for (std::size_t i = 0; i < used; ++i) {
    const double value = column.values[i];
    if (!std::isfinite(value))
      continue;

    minValue  = finiteCount == 0 ? value : std::min(minValue, value);
    maxValue  = finiteCount == 0 ? value : std::max(maxValue, value);
    sumValue += value;
    ++finiteCount;
  }

  m_blockQuery->bindValue(8, numeric ? 1 : 0);
  m_blockQuery->bindValue(9, finiteCount > 0 ? QVariant(minValue) : QVariant());
  m_blockQuery->bindValue(10, finiteCount > 0 ? QVariant(maxValue) : QVariant());
  m_blockQuery->bindValue(11, finiteCount > 0 ? QVariant(sumValue) : QVariant());
  m_blockQuery->bindValue(12, finiteCount);
  m_blockQuery->bindValue(13, valueBlob);
  m_blockQuery->bindValue(14, column.hasRaw ? rawBlob : QVariant());
  m_blockQuery->bindValue(15, column.hasText ? textBlob : QVariant());
  m_blockQuery->bindValue(16, column.hasRaw && column.hasText ? rawTextBlob : QVariant());
  m_blockQuery->bindValue(17, dtNs == 0 ? timesBlob : QVariant());

  if (!m_blockQuery->exec()) [[unlikely]] {
    qWarning() << "[SQLite] Insert block failed:" << m_blockQuery->lastError().text();
    return;
  }

  hashBlockRow(
    m_blocksHash, column.uniqueId, t0Ns, dtNs, block.samples, valueBlob, rawBlob, textBlob);
}

/**
 * @brief Processes a batch of published blocks into the SQLite database.
 */
void Sessions::ExportWorker::processItems(const std::vector<DataModel::DataBlockPtr>& items)
{
  if (items.empty())
    return;

  if (!m_dbOpen) {
    createDatabase(m_templateFrame);

    if (!m_dbOpen)
      return;

    m_steadyBaseline = items.front()->t0;
    resetMonotonicClock();
  }

  m_db->transaction();
  for (const auto& block : items)
    if (block && block->samples > 0)
      writeBlocks(block);

  if (!m_db->commit()) [[unlikely]] {
    qWarning() << "[Sessions::Export] commit() failed:" << m_db->lastError().text();
    m_db->rollback();
  }
}

/**
 * @brief Stores the schema template frame; must run on the worker thread (queued invoke) so the
 *        assignment never races processItems() or closeResources(). An empty frame is ignored:
 *        structure arrives asynchronously and QuickPlot has none until its first frame, so an
 *        empty payload landing second would wipe an adopted template and no file would be made.
 */
void Sessions::ExportWorker::setTemplateFrame(const DataModel::Frame& frame)
{
  if (frame.groups.empty())
    return;

  m_templateFrame = frame;
}

/**
 * @brief Adopts the structure the pipeline publishes when the connect-time fetch came back empty.
 *        QuickPlot derives its datasets from the first frame, so at connect there is nothing to
 *        fetch; blocks carry values only, so without this the file is never created. Ignored once
 *        a file exists -- an open file's schema is fixed for its lifetime.
 */
void Sessions::ExportWorker::applyPublishedStructure(const DataModel::Frame& frame)
{
  if (isResourceOpen() || !m_templateFrame.groups.empty())
    return;

  m_templateFrame = frame;
}

/**
 * @brief Creates the SQLite database file and writes the schema and session.
 */
void Sessions::ExportWorker::createDatabase(const DataModel::Frame& frame)
{
  SS_ASSERT(!m_dbOpen, return);

  const auto dbPath  = Sessions::DatabaseManager::canonicalDbPath(frame.title);
  const auto dirPath = QFileInfo(dbPath).absolutePath();
  QDir dir(dirPath);
  if (!dir.exists() && !dir.mkpath(".")) {
    qWarning() << "[SQLite] Failed to create directory:" << dirPath;
    return;
  }

  const auto dt          = QDateTime::currentDateTime();
  const QString connName = QStringLiteral("ss_sqlite_%1").arg(dt.toMSecsSinceEpoch());
  m_db.emplace(QSqlDatabase::addDatabase("QSQLITE", connName));
  m_db->setDatabaseName(dbPath);
  if (!m_db->open()) {
    qWarning() << "[SQLite] Cannot open database:" << m_db->lastError().text();
    m_db.reset();
    QSqlDatabase::removeDatabase(connName);
    return;
  }

  QSqlQuery pragma(*m_db);
  pragma.exec("PRAGMA journal_mode=WAL");
  pragma.exec("PRAGMA synchronous=NORMAL");
  pragma.exec("PRAGMA busy_timeout=5000");

  createSchema(pragma);

  insertSession(frame, dt);
  if (m_sessionId < 0) [[unlikely]] {
    qWarning() << "[SQLite] Aborting database open -- session row was not inserted";
    m_db->close();
    m_db.reset();
    QSqlDatabase::removeDatabase(connName);
    return;
  }

  writeColumnDefs(frame);

  storeProjectMetadata(frame);

  prepareHotpathQueries();

  m_dbOpen = true;
  Q_EMIT resourceOpenChanged();
}

/**
 * @brief Delegates to DatabaseManager::createSchema for a single source of truth.
 */
void Sessions::ExportWorker::createSchema(QSqlQuery& q)
{
  DatabaseManager::createSchema(q);
}

/**
 * @brief Inserts a new session row.
 */
void Sessions::ExportWorker::insertSession(const DataModel::Frame& frame, const QDateTime& dt)
{
  if (!m_db) [[unlikely]]
    return;

  const auto projectJson =
    QString::fromUtf8(QJsonDocument(buildReplayProjectJson(frame)).toJson(QJsonDocument::Compact));

  QSqlQuery q(*m_db);
  q.prepare("INSERT INTO sessions "
            "(project_title, started_at, project_json) "
            "VALUES (?, ?, ?)");
  q.bindValue(0, frame.title);
  q.bindValue(1, dt.toString(Qt::ISODate));
  q.bindValue(2, projectJson);

  if (!q.exec()) {
    qWarning() << "[SQLite] Insert session failed:" << q.lastError().text();
    return;
  }

  m_sessionId = q.lastInsertId().toInt();
  Q_EMIT sessionIdAssigned(m_sessionId);
  storeViewState();
}

/**
 * @brief Writes the controller's current view-state snapshot (spec 0062) into the open session
 *        row: at session start, on the controller's debounced pushes, and once more at close.
 *        Worker thread only; a closed database or unassigned session is a no-op.
 */
void Sessions::ExportWorker::storeViewState()
{
  if (m_sessionId < 0 || !m_db || !m_db->isOpen())
    return;

  QByteArray json;
  {
    QMutexLocker locker(m_projectSnapshotMutex);
    json = *m_viewStateSnapshot;
  }

  if (json.isEmpty())
    return;

  QSqlQuery q(*m_db);
  q.prepare("UPDATE sessions SET view_state = ? WHERE session_id = ?");
  q.bindValue(0, QString::fromUtf8(json));
  q.bindValue(1, m_sessionId);
  if (!q.exec())
    qWarning() << "[SQLite] view_state update failed:" << q.lastError().text();
}

/**
 * @brief Writes column definitions for the new session from the export schema.
 */
void Sessions::ExportWorker::writeColumnDefs(const DataModel::Frame& frame)
{
  DataModel::Frame projectFrame;
  bool haveProjectFrame = false;
  {
    QMutexLocker locker(m_projectSnapshotMutex);
    if (!m_projectSnapshot->isEmpty()) {
      const auto doc = QJsonDocument::fromJson(*m_projectSnapshot);
      if (doc.isObject())
        haveProjectFrame = DataModel::read(projectFrame, doc.object());
    }
  }

  m_schema = haveProjectFrame ? DataModel::buildExportSchema(projectFrame)
                              : DataModel::buildExportSchema(frame);

  if (m_schema.columns.empty()) {
    qWarning() << "[Sessions::Export] writeColumnDefs: schema has 0 columns --"
               << "frame has" << frame.groups.size() << "groups, project snapshot "
               << (haveProjectFrame ? "available" : "unavailable")
               << ". Check that the project's groups are populated before export starts.";
    return;
  }

  if (!m_db) [[unlikely]]
    return;

  QSqlQuery colQuery(*m_db);
  if (!colQuery.prepare("INSERT INTO columns (session_id, unique_id, source_id, source_title, "
                        "                     group_title, title, units, widget, is_virtual) "
                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)")) {
    qWarning() << "[Sessions::Export] columns INSERT prepare failed:"
               << colQuery.lastError().text();
    return;
  }

  m_db->transaction();
  int written = 0;
  for (const auto& col : m_schema.columns) {
    colQuery.bindValue(0, m_sessionId);
    colQuery.bindValue(1, col.uniqueId);
    colQuery.bindValue(2, col.sourceId);
    colQuery.bindValue(3, col.sourceTitle);
    colQuery.bindValue(4, col.groupTitle);
    colQuery.bindValue(5, col.title);
    colQuery.bindValue(6, col.units);
    colQuery.bindValue(7, col.widget);
    colQuery.bindValue(8, col.isVirtual ? 1 : 0);
    if (!colQuery.exec()) {
      qWarning() << "[Sessions::Export] columns INSERT failed for uid" << col.uniqueId << ":"
                 << colQuery.lastError().text();
      continue;
    }
    ++written;
  }
  m_db->commit();

  if (written == 0) {
    qWarning() << "[Sessions::Export] writeColumnDefs: 0 rows inserted out of"
               << m_schema.columns.size() << "-- report will be empty.";
  }
}

/**
 * @brief Stores the current project JSON in the project_metadata table.
 */
void Sessions::ExportWorker::storeProjectMetadata(const DataModel::Frame& frame)
{
  if (!m_db) [[unlikely]]
    return;

  const auto json =
    QString::fromUtf8(QJsonDocument(buildReplayProjectJson(frame)).toJson(QJsonDocument::Compact));
  const auto now = QDateTime::currentDateTime().toString(Qt::ISODate);

  m_db->transaction();
  QSqlQuery q(*m_db);

  q.prepare("INSERT OR REPLACE INTO project_metadata (key, value) VALUES ('project_json', ?)");
  q.bindValue(0, json);
  q.exec();

  q.prepare("INSERT OR REPLACE INTO project_metadata (key, value) VALUES ('project_title', ?)");
  q.bindValue(0, frame.title);
  q.exec();

  q.prepare("INSERT OR REPLACE INTO project_metadata (key, value) VALUES ('last_modified_at', ?)");
  q.bindValue(0, now);
  q.exec();

  q.prepare("INSERT INTO project_metadata (key, value) VALUES ('created_at', ?) "
            "ON CONFLICT(key) DO NOTHING");
  q.bindValue(0, now);
  q.exec();

  m_db->commit();
}

/**
 * @brief Produces the project JSON stored with the session.
 */
QJsonObject Sessions::ExportWorker::buildReplayProjectJson(const DataModel::Frame& frame) const
{
  {
    QMutexLocker locker(m_projectSnapshotMutex);
    if (!m_projectSnapshot->isEmpty()) {
      const auto doc = QJsonDocument::fromJson(*m_projectSnapshot);
      if (doc.isObject())
        return doc.object();
    }
  }

  static auto& appState = AppState::instance();
  if (appState.operationMode() == SerialStudio::ConsoleOnly) {
    QJsonObject json;
    json.insert(QStringLiteral("title"), frame.title);
    json.insert(QStringLiteral("operationMode"), QStringLiteral("ConsoleOnly"));
    return json;
  }

  QJsonObject json;
  json.insert(Keys::Title, frame.title);

  json.insert(Keys::FrameDetection, static_cast<int>(SerialStudio::EndDelimiterOnly));
  json.insert(Keys::FrameEnd, QStringLiteral("\\n"));
  json.insert(Keys::FrameStart, QStringLiteral(""));
  json.insert(Keys::Decoder, static_cast<int>(SerialStudio::PlainText));

  QJsonArray groupsArray;
  for (const auto& group : frame.groups)
    groupsArray.append(DataModel::serialize(group));

  json.insert(Keys::Groups, groupsArray);
  json.insert(Keys::Actions, QJsonArray());

  QJsonObject source;
  source.insert(Keys::Title, QStringLiteral("Device A"));
  source.insert(Keys::SourceId, 0);
  source.insert(Keys::BusType, static_cast<int>(SerialStudio::BusType::UART));
  source.insert(Keys::FrameDetection, static_cast<int>(SerialStudio::EndDelimiterOnly));
  source.insert(Keys::FrameStart, QString());
  source.insert(Keys::FrameEnd, QStringLiteral("\\n"));
  source.insert(Keys::Decoder, static_cast<int>(SerialStudio::PlainText));
  source.insert(Keys::FrameParserLanguage, static_cast<int>(SerialStudio::Native));

  const QString templateId = DataModel::defaultNativeTemplateId();
  source.insert(Keys::FrameParserTemplate, templateId);
  if (const auto* tmpl = DataModel::nativeTemplateById(templateId))
    source.insert(Keys::FrameParserParams, DataModel::nativeTemplateDefaults(*tmpl));

  QJsonArray sourcesArray;
  sourcesArray.append(source);
  json.insert(Keys::Sources, sourcesArray);

  return json;
}

/**
 * @brief Prepares the pre-compiled queries used on the frame hotpath.
 */
void Sessions::ExportWorker::prepareHotpathQueries()
{
  if (!m_db) [[unlikely]]
    return;

  m_blockQuery.emplace(*m_db);
  m_blockQuery->prepare(
    "INSERT INTO blocks "
    "(session_id, source_id, unique_id, block_number, t0_ns, t_end_ns, dt_ns, frames, "
    " is_numeric, min_value, max_value, sum_value, finite_count, "
    " values_blob, raw_values, texts, raw_texts, times) "
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

  m_rawBytesQuery.emplace(*m_db);
  m_rawBytesQuery->prepare("INSERT INTO raw_bytes (session_id, timestamp_ns, device_id, data) "
                           "VALUES (?, ?, ?, ?)");

  m_tableSnapshotQuery.emplace(*m_db);
  m_tableSnapshotQuery->prepare(
    "INSERT INTO table_snapshots "
    "(session_id, timestamp_ns, table_name, register_name, numeric_value, string_value) "
    "VALUES (?, ?, ?, ?, ?, ?)");
}

/**
 * @brief Drains the raw bytes queue and writes entries to the database.
 */
void Sessions::ExportWorker::writeRawBytes()
{
  SS_ASSERT_LOG(m_dbOpen);

  if (!m_db || !m_rawBytesQuery) [[unlikely]]
    return;

  constexpr size_t kMaxRawBatch = 1000;
  TimestampedRawBytes entry;
  size_t count = 0;

  m_db->transaction();
  while (count < kMaxRawBatch && m_rawQueue->try_dequeue(entry)) {
    if (!entry.data) [[unlikely]]
      continue;

    const auto elapsed = entry.data->timestamp - m_steadyBaseline;
    qint64 ns          = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();

    if (ns < 0) [[unlikely]]
      ns = 0;
    if (ns <= m_lastRawBytesNs)
      ns = m_lastRawBytesNs + 1;

    m_lastRawBytesNs = ns;

    const QByteArray chunk = entry.data->data;
    m_rawBytesQuery->bindValue(0, m_sessionId);
    m_rawBytesQuery->bindValue(1, ns);
    m_rawBytesQuery->bindValue(2, entry.deviceId);
    m_rawBytesQuery->bindValue(3, chunk);

    ++count;

    if (!m_rawBytesQuery->exec()) [[unlikely]] {
      qWarning() << "[SQLite] Insert raw_bytes failed:" << m_rawBytesQuery->lastError().text();
      continue;
    }

    hashRawChunk(m_rawHash, ns, entry.deviceId, chunk);
  }
  m_db->commit();
}

/**
 * @brief Drains the table snapshot queue and writes register changes to the database.
 */
void Sessions::ExportWorker::writeTableSnapshots()
{
  SS_ASSERT_LOG(m_dbOpen);

  if (!m_db || !m_tableSnapshotQuery) [[unlikely]]
    return;

  if (!m_snapshotQueue->peek())
    return;

  constexpr size_t kMaxSnapshotBatch = 1000;
  TableSnapshotEntry entry;
  size_t count = 0;

  m_db->transaction();
  while (count < kMaxSnapshotBatch && m_snapshotQueue->try_dequeue(entry)) {
    const auto elapsed = entry.timestamp - m_steadyBaseline;
    qint64 ns          = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    if (ns < 0) [[unlikely]]
      ns = 0;

    const auto& value = entry.value;
    m_tableSnapshotQuery->bindValue(0, m_sessionId);
    m_tableSnapshotQuery->bindValue(1, ns);
    m_tableSnapshotQuery->bindValue(2, entry.tableName);
    m_tableSnapshotQuery->bindValue(3, entry.registerName);
    m_tableSnapshotQuery->bindValue(4, value.isNumeric ? QVariant(value.numericValue) : QVariant());
    m_tableSnapshotQuery->bindValue(5, value.isNumeric ? QVariant() : QVariant(value.stringValue));

    if (!m_tableSnapshotQuery->exec()) [[unlikely]]
      qWarning() << "[SQLite] Insert table_snapshots failed:"
                 << m_tableSnapshotQuery->lastError().text();

    ++count;
  }
  m_db->commit();
}

/**
 * @brief Updates the session's ended_at timestamp and stamps the spec-0044 fingerprint,
 *        version, classification, and link-loss columns.
 */
void Sessions::ExportWorker::finalizeSession()
{
  if (m_sessionId < 0 || !m_db || !m_db->isOpen())
    return;

  storeViewState();

  QSqlQuery q(*m_db);
  q.prepare("UPDATE sessions SET ended_at = ?, raw_sha256 = ?, readings_sha256 = ?, "
            "stream_sha256 = ?, app_version = ?, capture_format = ?, repro_class = ?, "
            "frames_dropped = ?, overflow_bytes = ? WHERE session_id = ?");
  q.bindValue(0, QDateTime::currentDateTime().toString(Qt::ISODate));
  q.bindValue(1, QString::fromLatin1(m_rawHash.result().toHex()));
  q.bindValue(2, QString::fromLatin1(m_blocksHash.result().toHex()));
  q.bindValue(3, QVariant(QMetaType(QMetaType::QString)));
  q.bindValue(4, QStringLiteral(APP_VERSION));
  q.bindValue(5, DatabaseManager::kCaptureFormatVersion);
  q.bindValue(6, buildReproClassJson());
  q.bindValue(7, static_cast<qint64>(m_linkDroppedFrames->load(std::memory_order_relaxed)));
  q.bindValue(8, static_cast<qint64>(m_linkOverflowBytes->load(std::memory_order_relaxed)));
  q.bindValue(9, m_sessionId);
  if (q.exec())
    return;

  qWarning() << "[SQLite] Session finalize failed:" << q.lastError().text();

  QSqlQuery fallback(*m_db);
  fallback.prepare("UPDATE sessions SET ended_at = ? WHERE session_id = ?");
  fallback.bindValue(0, QDateTime::currentDateTime().toString(Qt::ISODate));
  fallback.bindValue(1, m_sessionId);
  if (!fallback.exec())
    qWarning() << "[SQLite] Session ended_at fallback failed:" << fallback.lastError().text();
}

/**
 * @brief Returns true when any dataset in the project JSON carries a transform script.
 */
static bool projectHasTransforms(const QJsonObject& project)
{
  const auto groups = project.value(Keys::Groups).toArray();
  for (const auto& groupRef : groups) {
    const auto datasets = groupRef.toObject().value(Keys::Datasets).toArray();
    for (const auto& datasetRef : datasets)
      if (!datasetRef.toObject().value(Keys::TransformCode).toString().isEmpty())
        return true;
  }

  return false;
}

/**
 * @brief Builds the reproducibility classification JSON stored with the session: features whose
 *        interpretation inputs are not archived make the session (or dataset) not mechanically
 *        verifiable, so the verifier reports the reason instead of a false verdict.
 */
QString Sessions::ExportWorker::buildReproClassJson() const
{
  QByteArray snapshot;
  {
    QMutexLocker locker(m_projectSnapshotMutex);
    snapshot = *m_projectSnapshot;
  }

  bool transformsPresent = false;
  bool tablesPresent     = false;
  if (!snapshot.isEmpty()) {
    const auto project = QJsonDocument::fromJson(snapshot).object();
    tablesPresent      = !project.value(Keys::Tables).toArray().isEmpty();
    transformsPresent  = projectHasTransforms(project);
  }

  QJsonArray virtualIds;

  for (const auto& col : m_schema.columns)
    if (col.isVirtual)
      virtualIds.append(col.uniqueId);

  QJsonObject json;
  json.insert(QStringLiteral("controlScript"),
              m_controlScriptSeen->load(std::memory_order_relaxed));
  json.insert(QStringLiteral("transformsPresent"), transformsPresent);
  json.insert(QStringLiteral("tablesPresent"), tablesPresent);
  json.insert(QStringLiteral("virtualDatasets"), virtualIds);
  return QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact));
}

//--------------------------------------------------------------------------------------------------
// Export singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the SQLite export manager.
 */
Sessions::Export::Export()
  : DataModel::FrameConsumer<DataModel::DataBlockPtr>(
      DataModel::FrameConsumerConfig{8192, 1024, 1000})
  , m_isOpen(false)
  , m_exportEnabled(false)
  , m_currentSessionId(-1)
  , m_persistSettings(true)
  , m_rawBytesQueue(8192)
  , m_tableSnapshotQueue(1024)
  , m_operationMode(static_cast<int>(AppState::instance().operationMode()))
  , m_controlScriptSeen(false)
  , m_linkDroppedFrames(0)
  , m_linkOverflowBytes(0)
  , m_lastLinkDroppedSample(0)
  , m_lastLinkOverflowSample(0)
  , m_appState(&AppState::instance())
  , m_projectModel(&DataModel::ProjectModel::instance())
  , m_frameBuilder(&DataModel::FrameBuilder::instance())
  , m_controlScript(nullptr)
  , m_connectionManager(nullptr)
  , m_dashboard(nullptr)
{
  initializeWorker();

  static auto& lemonSqueezy = Licensing::LemonSqueezy::instance();
  connect(&lemonSqueezy, &Licensing::LemonSqueezy::activatedChanged, this, [this] {
    if (exportEnabled()
        && (!Licensing::CommercialToken::current().isValid() || !SS_LICENSE_GUARD()))
      setExportEnabled(false);
  });
}

/**
 * @brief Default destructor.
 */
Sessions::Export::~Export() = default;

/**
 * @brief Returns the singleton instance.
 */
Sessions::Export& Sessions::Export::instance()
{
  static Export singleton;
  return singleton;
}

/**
 * @brief Returns whether the database is currently open.
 */
bool Sessions::Export::isOpen() const
{
  return m_isOpen.load(std::memory_order_relaxed);
}

/**
 * @brief Returns whether SQLite export is enabled.
 */
bool Sessions::Export::exportEnabled() const
{
  return m_exportEnabled.load(std::memory_order_relaxed);
}

/**
 * @brief Returns the row id of the session currently being recorded, or -1.
 */
int Sessions::Export::currentSessionId() const
{
  return m_currentSessionId.load(std::memory_order_relaxed);
}

/**
 * @brief Closes the current database file. The worker is asked unconditionally: m_isOpen only
 *        catches up through a queued signal, so a caller that opened and closed within one
 *        event-loop turn (the spec-0047 dual replay does exactly that) would otherwise leave the
 *        file open and append the next pass into it.
 */
void Sessions::Export::closeFile()
{
  closeWorkerResources();

  if (m_isOpen.exchange(false, std::memory_order_relaxed))
    Q_EMIT openChanged();

  m_lastTableSnapshot.clear();

  if (m_currentSessionId.exchange(-1, std::memory_order_relaxed) != -1)
    Q_EMIT currentSessionIdChanged();
}

/**
 * @brief Wires external signals for auto-close on disconnect.
 */
void Sessions::Export::setupExternalConnections()
{
  connect(
    m_frameBuilder,
    &DataModel::FrameBuilder::structurePublished,
    this,
    [this](int, const DataModel::Frame& frame) {
      auto* worker = static_cast<ExportWorker*>(m_worker);
      SS_ASSERT(worker != nullptr, return);

      QMetaObject::invokeMethod(
        worker, [worker, frame] { worker->applyPublishedStructure(frame); }, Qt::QueuedConnection);
    });
  m_controlScript     = &DataModel::ControlScript::instance();
  m_connectionManager = &IO::ConnectionManager::instance();

  connect(&AppState::instance(), &AppState::operationModeChanged, this, [this] {
    const auto mode = AppState::instance().operationMode();
    m_operationMode.store(static_cast<int>(mode), std::memory_order_relaxed);

    if (isOpen())
      closeFile();

    if (mode == SerialStudio::ConsoleOnly && exportEnabled())
      setExportEnabled(false);
  });

  connect(m_frameBuilder,
          &DataModel::FrameBuilder::sessionStructureReady,
          this,
          [this](const DataModel::Frame& frame) {
            m_sessionStructure = frame;
            refreshTemplateFrame();
          });

  connect(m_connectionManager, &IO::ConnectionManager::connectedChanged, this, [this] {
    if (!m_connectionManager->isConnected())
      closeFile();
  });

  connect(&CSV::Player::instance(), &CSV::Player::openChanged, this, [this] {
    if (CSV::Player::instance().isOpen())
      closeFile();
  });

  connect(&MDF4::Player::instance(), &MDF4::Player::openChanged, this, [this] {
    if (MDF4::Player::instance().isOpen())
      closeFile();
  });

  auto& pm = DataModel::ProjectModel::instance();
  connect(&pm,
          &DataModel::ProjectModel::jsonFileChanged,
          this,
          &Sessions::Export::refreshProjectSnapshot);
  connect(
    &pm, &DataModel::ProjectModel::groupsChanged, this, &Sessions::Export::refreshProjectSnapshot);
  connect(
    &pm, &DataModel::ProjectModel::actionsChanged, this, &Sessions::Export::refreshProjectSnapshot);
  connect(
    &pm, &DataModel::ProjectModel::sourcesChanged, this, &Sessions::Export::refreshProjectSnapshot);
  connect(
    &pm, &DataModel::ProjectModel::tablesChanged, this, &Sessions::Export::refreshProjectSnapshot);
  connect(&pm,
          &DataModel::ProjectModel::editorWorkspacesChanged,
          this,
          &Sessions::Export::refreshProjectSnapshot);

  connect(&pm,
          &DataModel::ProjectModel::sourceFrameParserCodeChanged,
          this,
          &Sessions::Export::refreshProjectSnapshot);
  connect(&pm,
          &DataModel::ProjectModel::sourceFrameParserLanguageChanged,
          this,
          &Sessions::Export::refreshProjectSnapshot);
  connect(&pm,
          &DataModel::ProjectModel::sourceFrameParserTemplateChanged,
          this,
          &Sessions::Export::refreshProjectSnapshot);
  connect(&pm,
          &DataModel::ProjectModel::sourceFrameParserParamsChanged,
          this,
          &Sessions::Export::refreshProjectSnapshot);

  connect(&AppState::instance(),
          &AppState::operationModeChanged,
          this,
          &Sessions::Export::refreshProjectSnapshot);

  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::timeout1Hz,
          this,
          &Sessions::Export::captureTableSnapshots);

  wireViewState();
  refreshProjectSnapshot();

  const bool persisted = m_settings.value("SQLiteExport/Enabled", false).toBool();
  setExportEnabled(persisted);
}

/**
 * @brief Wires the spec-0062 view-state capture: the dashboard's change signal into the snapshot,
 *        and a 1.5 s single-shot debounce into the worker push.
 */
void Sessions::Export::wireViewState()
{
  // code-verify off
  // This IS the setupExternalConnections capture the rule asks for: Dashboard is built last in
  // the composition root, so a ctor-init-list capture would recurse the Meyers guard and abort.
  m_dashboard = &UI::Dashboard::instance();
  // code-verify on
  m_viewStateDebounce.setSingleShot(true);
  m_viewStateDebounce.setInterval(kViewStateDebounceMs);
  connect(&m_viewStateDebounce, &QTimer::timeout, this, &Sessions::Export::pushViewStateToWorker);
  connect(m_dashboard,
          &UI::Dashboard::viewStateChanged,
          this,
          &Sessions::Export::refreshViewStateSnapshot);
  refreshViewStateSnapshot();
}

/**
 * @brief Main-thread-only: snapshots the dashboard view state (spec 0062) beside the project
 *        snapshot and arms the debounce that hands it to the worker while a session is open.
 */
void Sessions::Export::refreshViewStateSnapshot()
{
  SS_ASSERT(m_dashboard != nullptr, return);
  QByteArray payload = m_dashboard->viewStateJson().toUtf8();

  {
    QMutexLocker locker(&m_projectSnapshotMutex);
    m_viewStateSnapshot = std::move(payload);
  }

  if (isOpen())
    m_viewStateDebounce.start();
}

/**
 * @brief Debounced: asks the worker to persist the current view-state snapshot.
 */
void Sessions::Export::pushViewStateToWorker()
{
  auto* worker = static_cast<ExportWorker*>(m_worker);
  if (!worker || !isOpen())
    return;

  QMetaObject::invokeMethod(worker, &ExportWorker::storeViewState, Qt::QueuedConnection);
}

/**
 * @brief Main-thread-only: snapshots ProjectModel into m_projectSnapshot.
 */
void Sessions::Export::refreshProjectSnapshot()
{
  SS_ASSERT(m_appState != nullptr, return);
  SS_ASSERT(m_projectModel != nullptr, return);

  QByteArray payload;
  if (m_appState->operationMode() == SerialStudio::ProjectFile) {
    if (!m_projectModel->groups().empty()) {
      const auto doc = QJsonDocument(m_projectModel->serializeToJson());
      payload        = doc.toJson(QJsonDocument::Compact);
    }
  }

  QMutexLocker locker(&m_projectSnapshotMutex);
  m_projectSnapshot = std::move(payload);
}

/**
 * @brief Enables or disables SQLite export.
 */
void Sessions::Export::setExportEnabled(const bool enabled)
{
  SS_ASSERT(m_appState != nullptr, return);

  const auto& tk      = Licensing::CommercialToken::current();
  const bool licensed = tk.isValid() && SS_LICENSE_GUARD();

  const bool allow =
    enabled && licensed && m_appState->operationMode() != SerialStudio::ConsoleOnly;

  if (m_exportEnabled.load(std::memory_order_relaxed) == allow)
    return;

  if (!allow)
    closeFile();

  m_exportEnabled.store(allow, std::memory_order_relaxed);
  setConsumerEnabled(allow);
  if (m_persistSettings)
    m_settings.setValue("SQLiteExport/Enabled", allow);

  Q_EMIT enabledChanged();
}

/**
 * @brief Toggles whether export-enabled changes get written to QSettings.
 */
void Sessions::Export::setSettingsPersistent(const bool persistent)
{
  m_persistSettings = persistent;
}

/**
 * @brief Re-baselines the link-loss delta accumulators and the control-script sticky flag at
 *        session open, so pre-session drops never count against the new session.
 */
void Sessions::Export::resetSessionHealthBaseline()
{
  SS_ASSERT(m_controlScript != nullptr, return);
  SS_ASSERT(m_connectionManager != nullptr, return);

  const auto stats         = m_connectionManager->linkStats();
  m_lastLinkDroppedSample  = stats.droppedFrames;
  m_lastLinkOverflowSample = stats.overflowBytes;
  m_linkDroppedFrames.store(0, std::memory_order_relaxed);
  m_linkOverflowBytes.store(0, std::memory_order_relaxed);
  m_controlScriptSeen.store(m_controlScript->running(), std::memory_order_relaxed);
}

/**
 * @brief Main-thread 1 Hz sample of the session-health classification inputs: sticky
 *        control-script flag and link-loss counter deltas (a decrease means a reader reset).
 */
void Sessions::Export::sampleSessionHealth()
{
  SS_ASSERT(m_controlScript != nullptr, return);
  SS_ASSERT(m_connectionManager != nullptr, return);

  if (m_controlScript->running())
    m_controlScriptSeen.store(true, std::memory_order_relaxed);

  const auto stats = m_connectionManager->linkStats();

  const quint64 droppedDelta  = stats.droppedFrames >= m_lastLinkDroppedSample
                                ? stats.droppedFrames - m_lastLinkDroppedSample
                                : stats.droppedFrames;
  const quint64 overflowDelta = stats.overflowBytes >= m_lastLinkOverflowSample
                                ? stats.overflowBytes - m_lastLinkOverflowSample
                                : stats.overflowBytes;

  m_lastLinkDroppedSample  = stats.droppedFrames;
  m_lastLinkOverflowSample = stats.overflowBytes;
  m_linkDroppedFrames.fetch_add(droppedDelta, std::memory_order_relaxed);
  m_linkOverflowBytes.fetch_add(overflowDelta, std::memory_order_relaxed);
}

/**
 * @brief Main-thread 1 Hz poll: diffs the live data-table store against the last captured
 *        state and enqueues changed registers for the table_snapshots table.
 */
void Sessions::Export::captureTableSnapshots()
{
  SS_ASSERT(m_frameBuilder != nullptr, return);

  if (!exportEnabled() || !isOpen() || SerialStudio::isAnyPlayerOpen()) {
    m_lastTableSnapshot.clear();
    return;
  }

  sampleSessionHealth();

  bool initialized = false;
  decltype(m_frameBuilder->tableStore().snapshot()) snapshot;
  m_frameBuilder->invokeOnBuilderThreadBlocking([&] {
    const auto& store = m_frameBuilder->tableStore();
    initialized       = store.isInitialized();
    if (initialized)
      snapshot = store.snapshot();
  });

  if (!initialized)
    return;

  const auto changed =
    [this](const QString& table, const QString& reg, const DataModel::RegisterValue& val) {
      const auto t = m_lastTableSnapshot.constFind(table);
      if (t == m_lastTableSnapshot.constEnd())
        return true;

      const auto r = t.value().constFind(reg);
      if (r == t.value().constEnd())
        return true;

      return r.value().isNumeric != val.isNumeric || r.value().numericValue != val.numericValue
          || r.value().stringValue != val.stringValue;
    };

  const auto now = DataModel::TimestampedFrame::SteadyClock::now();
  for (auto t = snapshot.constBegin(); t != snapshot.constEnd(); ++t) {
    if (t.key() == DataModel::systemDataTableName())
      continue;

    for (auto r = t.value().constBegin(); r != t.value().constEnd(); ++r) {
      if (!changed(t.key(), r.key(), r.value()))
        continue;

      TableSnapshotEntry entry;
      entry.timestamp    = now;
      entry.tableName    = t.key();
      entry.registerName = r.key();
      entry.value        = r.value();
      m_tableSnapshotQueue.try_enqueue(std::move(entry));
    }
  }

  m_lastTableSnapshot = snapshot;
}

/**
 * @brief Enqueues one published block for SQLite export. The single producer for this SPSC queue
 *        is the pipeline thread, for both lanes (spec 0055 D8).
 */
void Sessions::Export::ingestBlock(const DataModel::DataBlockPtr& block)
{
  if (!block || !m_exportEnabled.load(std::memory_order_relaxed))
    return;

  if (SerialStudio::isAnyPlayerOpen())
    return;

  enqueueData(block);
}

/**
 * @brief Captures the current project schema frame and queues it to the worker: a block carries
 *        dataset identities but no structure, so the column definitions and the archived project
 *        JSON have to come from here.
 */
void Sessions::Export::refreshTemplateFrame()
{
  SS_ASSERT(m_worker != nullptr, return);

  auto* worker = static_cast<ExportWorker*>(m_worker);
  QMetaObject::invokeMethod(
    worker,
    [worker, frame = m_sessionStructure] { worker->setTemplateFrame(frame); },
    Qt::QueuedConnection);
}

/**
 * @brief Enqueues raw console bytes for the raw_bytes table.
 */
void Sessions::Export::hotpathTxRawBytes(int deviceId, const IO::CapturedDataPtr& data)
{
  if (!m_exportEnabled.load(std::memory_order_relaxed))
    return;

  TimestampedRawBytes entry;
  entry.deviceId = deviceId;
  entry.data     = data;
  m_rawBytesQueue.try_enqueue(std::move(entry));
}

/**
 * @brief Factory method for the worker.
 */
DataModel::FrameConsumerWorkerBase* Sessions::Export::createWorker()
{
  auto* w = new ExportWorker(&m_pendingQueue,
                             &m_consumerEnabled,
                             &m_queueSize,
                             &m_rawBytesQueue,
                             &m_tableSnapshotQueue,
                             &m_operationMode,
                             &m_projectSnapshotMutex,
                             &m_projectSnapshot,
                             &m_viewStateSnapshot,
                             &m_controlScriptSeen,
                             &m_linkDroppedFrames,
                             &m_linkOverflowBytes);
  connect(w,
          &DataModel::FrameConsumerWorkerBase::resourceOpenChanged,
          this,
          &Export::onWorkerOpenChanged);
  connect(w,
          &ExportWorker::sessionIdAssigned,
          this,
          &Export::onWorkerSessionIdAssigned,
          Qt::QueuedConnection);
  return w;
}

/**
 * @brief Mirrors the worker's freshly-assigned session id onto the controller.
 */
void Sessions::Export::onWorkerSessionIdAssigned(int sessionId)
{
  if (m_currentSessionId.exchange(sessionId, std::memory_order_relaxed) != sessionId)
    Q_EMIT currentSessionIdChanged();
}

/**
 * @brief Updates the isOpen state when the worker opens/closes the database.
 */
void Sessions::Export::onWorkerOpenChanged()
{
  auto* worker     = static_cast<ExportWorker*>(m_worker);
  const bool state = worker->isResourceOpen();
  if (m_isOpen.load(std::memory_order_relaxed) != state) {
    if (state)
      resetSessionHealthBaseline();

    m_isOpen.store(state, std::memory_order_relaxed);
    Q_EMIT openChanged();
  }

  if (!state && m_currentSessionId.exchange(-1, std::memory_order_relaxed) != -1)
    Q_EMIT currentSessionIdChanged();
}

#endif  // BUILD_COMMERCIAL

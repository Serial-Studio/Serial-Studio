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

#  include "Sessions/Export.h"

#  include <QDateTime>
#  include <QDir>
#  include <QJsonArray>
#  include <QJsonDocument>
#  include <QJsonObject>
#  include <QSqlError>

#  include "AppState.h"
#  include "CSV/Player.h"
#  include "DataModel/FrameBuilder.h"
#  include "DataModel/FrameParser.h"
#  include "DataModel/ProjectModel.h"
#  include "IO/ConnectionManager.h"
#  include "Licensing/LemonSqueezy.h"
#  include "MDF4/Player.h"
#  include "Misc/WorkspaceManager.h"
#  include "Sessions/DatabaseManager.h"

//--------------------------------------------------------------------------------------------------
// ExportWorker implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the SQLite export worker with both frame and raw bytes queues.
 */
Sessions::ExportWorker::ExportWorker(
  moodycamel::ReaderWriterQueue<DataModel::TimestampedFramePtr>* frameQueue,
  std::atomic<bool>* enabled,
  std::atomic<size_t>* queueSize,
  moodycamel::ReaderWriterQueue<TimestampedRawBytes>* rawQueue,
  std::atomic<int>* operationMode,
  QMutex* projectSnapshotMutex,
  const QByteArray* projectSnapshot)
  : DataModel::FrameConsumerWorker<DataModel::TimestampedFramePtr>(frameQueue, enabled, queueSize)
  , m_dbOpen(false)
  , m_sessionId(-1)
  , m_lastRawBytesNs(-1)
  , m_rawQueue(rawQueue)
  , m_operationMode(operationMode)
  , m_projectSnapshotMutex(projectSnapshotMutex)
  , m_projectSnapshot(projectSnapshot)
{
  Q_ASSERT(rawQueue);
  Q_ASSERT(operationMode);
  Q_ASSERT(projectSnapshotMutex);
  Q_ASSERT(projectSnapshot);
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

  m_readingQuery.clear();
  m_rawBytesQuery.clear();
  m_tableSnapshotQuery.clear();

  const auto connName = m_db.connectionName();
  m_db.close();
  m_db = QSqlDatabase();
  QSqlDatabase::removeDatabase(connName);

  m_dbOpen         = false;
  m_sessionId      = -1;
  m_lastRawBytesNs = -1;
  m_schema         = DataModel::ExportSchema{};
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
  // Drain the frame queue via base class
  DataModel::FrameConsumerWorker<DataModel::TimestampedFramePtr>::processData();

  // Honour the base class' enabled gate
  if (!consumerEnabled())
    return;

  // Console-only has no frames — open schema-less DB when raw bytes arrive
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

  // Drain the raw bytes queue
  if (m_dbOpen)
    writeRawBytes();
}

/**
 * @brief Processes a batch of timestamped frames into the SQLite database.
 */
void Sessions::ExportWorker::processItems(const std::vector<DataModel::TimestampedFramePtr>& items)
{
  if (items.empty())
    return;

  // Lazily create the database on the first incoming batch
  if (!m_dbOpen) {
    createDatabase(items.front()->data);

    if (!m_dbOpen)
      return;

    m_steadyBaseline = items.front()->timestamp;
    resetMonotonicClock();
  }

  // Batch writes into a single transaction
  m_db.transaction();
  for (const auto& frame : items) {
    const qint64 ns = monotonicFrameNs(frame->timestamp, m_steadyBaseline);
    for (const auto& group : frame->data.groups) {
      for (const auto& dataset : group.datasets) {
        m_readingQuery.bindValue(0, m_sessionId);
        m_readingQuery.bindValue(1, ns);
        m_readingQuery.bindValue(2, dataset.uniqueId);
        m_readingQuery.bindValue(3, dataset.rawNumericValue);
        m_readingQuery.bindValue(4, dataset.rawValue);
        m_readingQuery.bindValue(5, dataset.numericValue);
        m_readingQuery.bindValue(6, dataset.value);
        m_readingQuery.bindValue(7, dataset.isNumeric ? 1 : 0);

        if (!m_readingQuery.exec()) [[unlikely]]
          qWarning() << "[SQLite] Insert reading failed:" << m_readingQuery.lastError().text();
      }
    }
  }

  // Roll back on failure
  if (!m_db.commit()) [[unlikely]] {
    qWarning() << "[Sessions::Export] commit() failed:" << m_db.lastError().text();
    m_db.rollback();
  }
}

/**
 * @brief Creates the SQLite database file and writes the schema and session.
 */
void Sessions::ExportWorker::createDatabase(const DataModel::Frame& frame)
{
  Q_ASSERT(!m_dbOpen);

  // Use the canonical per-project path
  const auto dbPath  = Sessions::DatabaseManager::canonicalDbPath(frame.title);
  const auto dirPath = QFileInfo(dbPath).absolutePath();
  QDir dir(dirPath);
  if (!dir.exists() && !dir.mkpath(".")) {
    qWarning() << "[SQLite] Failed to create directory:" << dirPath;
    return;
  }

  // Open or reuse the database
  const auto dt          = QDateTime::currentDateTime();
  const QString connName = QStringLiteral("ss_sqlite_%1").arg(dt.toMSecsSinceEpoch());
  m_db                   = QSqlDatabase::addDatabase("QSQLITE", connName);
  m_db.setDatabaseName(dbPath);
  if (!m_db.open()) {
    qWarning() << "[SQLite] Cannot open database:" << m_db.lastError().text();
    return;
  }

  // Performance pragmas
  QSqlQuery pragma(m_db);
  pragma.exec("PRAGMA journal_mode=WAL");
  pragma.exec("PRAGMA synchronous=NORMAL");
  pragma.exec("PRAGMA busy_timeout=5000");

  // Create every table the session format expects
  createSchema(pragma);

  // Insert the new session row — abandon the open on failure
  insertSession(frame, dt);
  if (m_sessionId < 0) [[unlikely]] {
    qWarning() << "[SQLite] Aborting database open — session row was not inserted";
    const auto connName = m_db.connectionName();
    m_db.close();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connName);
    return;
  }

  // Build export schema and write column definitions
  writeColumnDefs(frame);

  // Store the project JSON in project_metadata for offline reconstruction
  storeProjectMetadata(frame);

  // Prepare reusable queries for the hotpath
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
  // Serialize a replayable project JSON
  const auto projectJson =
    QString::fromUtf8(QJsonDocument(buildReplayProjectJson(frame)).toJson(QJsonDocument::Compact));

  QSqlQuery q(m_db);
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
}

/**
 * @brief Writes column definitions for the new session from the export schema.
 */
void Sessions::ExportWorker::writeColumnDefs(const DataModel::Frame& frame)
{
  m_schema = DataModel::buildExportSchema(frame);

  QSqlQuery colQuery(m_db);
  colQuery.prepare(
    "INSERT INTO columns (session_id, unique_id, group_title, title, units, widget, is_virtual) "
    "VALUES (?, ?, ?, ?, ?, ?, ?)");

  m_db.transaction();
  for (const auto& col : m_schema.columns) {
    colQuery.bindValue(0, m_sessionId);
    colQuery.bindValue(1, col.uniqueId);
    colQuery.bindValue(2, col.groupTitle);
    colQuery.bindValue(3, col.title);
    colQuery.bindValue(4, col.units);
    colQuery.bindValue(5, col.widget);
    colQuery.bindValue(6, col.isVirtual ? 1 : 0);
    colQuery.exec();
  }
  m_db.commit();
}

/**
 * @brief Stores the current project JSON in the project_metadata table.
 */
void Sessions::ExportWorker::storeProjectMetadata(const DataModel::Frame& frame)
{
  // Same replayable snapshot the session row uses
  const auto json =
    QString::fromUtf8(QJsonDocument(buildReplayProjectJson(frame)).toJson(QJsonDocument::Compact));
  const auto now = QDateTime::currentDateTime().toString(Qt::ISODate);

  m_db.transaction();
  QSqlQuery q(m_db);

  q.prepare("INSERT OR REPLACE INTO project_metadata (key, value) VALUES ('project_json', ?)");
  q.bindValue(0, json);
  q.exec();

  q.prepare("INSERT OR REPLACE INTO project_metadata (key, value) VALUES ('project_title', ?)");
  q.bindValue(0, frame.title);
  q.exec();

  q.prepare("INSERT OR REPLACE INTO project_metadata (key, value) VALUES ('last_modified_at', ?)");
  q.bindValue(0, now);
  q.exec();

  q.prepare("INSERT OR IGNORE INTO project_metadata (key, value) VALUES ('created_at', ?)");
  q.bindValue(0, now);
  q.exec();

  m_db.commit();
}

/**
 * @brief Produces the project JSON stored with the session.
 */
QJsonObject Sessions::ExportWorker::buildReplayProjectJson(const DataModel::Frame& frame) const
{
  // Prefer the main-thread snapshot (ProjectModel is not thread-safe)
  {
    QMutexLocker locker(m_projectSnapshotMutex);
    if (!m_projectSnapshot->isEmpty()) {
      const auto doc = QJsonDocument::fromJson(*m_projectSnapshot);
      if (doc.isObject())
        return doc.object();
    }
  }

  // ConsoleOnly has no parsed structure
  if (AppState::instance().operationMode() == SerialStudio::ConsoleOnly) {
    QJsonObject json;
    json.insert(QStringLiteral("title"), frame.title);
    json.insert(QStringLiteral("operationMode"), QStringLiteral("ConsoleOnly"));
    return json;
  }

  // QuickPlot fallback — synthesise a minimal project from the frame
  QJsonObject json;
  json.insert(Keys::Title, frame.title);

  // Frame detection settings (QuickPlot line-based CSV)
  json.insert(Keys::FrameDetection, static_cast<int>(SerialStudio::EndDelimiterOnly));
  json.insert(Keys::FrameEnd, QStringLiteral("\\n"));
  json.insert(Keys::FrameStart, QStringLiteral(""));
  json.insert(Keys::Decoder, static_cast<int>(SerialStudio::PlainText));

  // Groups from the live frame
  QJsonArray groupsArray;
  for (const auto& group : frame.groups)
    groupsArray.append(DataModel::serialize(group));

  json.insert(Keys::Groups, groupsArray);
  json.insert(Keys::Actions, QJsonArray());

  // Build a default source with a working parser (prefer Lua, fall back to JS)
  int parserLanguage = static_cast<int>(SerialStudio::Lua);
  QString parserCode = DataModel::FrameParser::defaultTemplateCode(SerialStudio::Lua);
  if (parserCode.isEmpty()) {
    parserLanguage = static_cast<int>(SerialStudio::JavaScript);
    parserCode     = DataModel::FrameParser::defaultTemplateCode(SerialStudio::JavaScript);
  }

  QJsonObject source;
  source.insert(Keys::Title, QStringLiteral("Device A"));
  source.insert(Keys::SourceId, 0);
  source.insert(Keys::BusType, static_cast<int>(SerialStudio::BusType::UART));
  source.insert(Keys::FrameDetection, static_cast<int>(SerialStudio::EndDelimiterOnly));
  source.insert(Keys::FrameStart, QString());
  source.insert(Keys::FrameEnd, QStringLiteral("\\n"));
  source.insert(Keys::Decoder, static_cast<int>(SerialStudio::PlainText));
  source.insert(Keys::FrameParserLanguage, parserLanguage);
  source.insert(Keys::FrameParserCode, parserCode);

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
  // Readings (one row per dataset per frame)
  m_readingQuery = QSqlQuery(m_db);
  m_readingQuery.prepare(
    "INSERT INTO readings "
    "(session_id, timestamp_ns, unique_id, raw_numeric_value, raw_string_value, "
    " final_numeric_value, final_string_value, is_numeric) "
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

  // Raw device bytes (separate stream, batched on its own queue)
  m_rawBytesQuery = QSqlQuery(m_db);
  m_rawBytesQuery.prepare("INSERT INTO raw_bytes (session_id, timestamp_ns, device_id, data) "
                          "VALUES (?, ?, ?, ?)");

  // Snapshots of user data tables taken per frame
  m_tableSnapshotQuery = QSqlQuery(m_db);
  m_tableSnapshotQuery.prepare(
    "INSERT INTO table_snapshots "
    "(session_id, timestamp_ns, table_name, register_name, numeric_value, string_value) "
    "VALUES (?, ?, ?, ?, ?, ?)");
}

/**
 * @brief Drains the raw bytes queue and writes entries to the database.
 */
void Sessions::ExportWorker::writeRawBytes()
{
  Q_ASSERT(m_dbOpen);

  constexpr size_t kMaxRawBatch = 1000;
  TimestampedRawBytes entry;
  size_t count = 0;

  m_db.transaction();
  while (count < kMaxRawBatch && m_rawQueue->try_dequeue(entry)) {
    const auto elapsed = entry.data->timestamp - m_steadyBaseline;
    qint64 ns          = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();

    // Clamp pre-baseline ns, then force strict monotonicity
    if (ns < 0) [[unlikely]]
      ns = 0;
    if (ns <= m_lastRawBytesNs)
      ns = m_lastRawBytesNs + 1;
    m_lastRawBytesNs = ns;

    m_rawBytesQuery.bindValue(0, m_sessionId);
    m_rawBytesQuery.bindValue(1, ns);
    m_rawBytesQuery.bindValue(2, entry.deviceId);
    m_rawBytesQuery.bindValue(3, entry.data && entry.data->data ? *entry.data->data : QByteArray());

    if (!m_rawBytesQuery.exec()) [[unlikely]]
      qWarning() << "[SQLite] Insert raw_bytes failed:" << m_rawBytesQuery.lastError().text();

    ++count;
  }
  m_db.commit();
}

/**
 * @brief Updates the session's ended_at timestamp.
 */
void Sessions::ExportWorker::finalizeSession()
{
  if (m_sessionId < 0 || !m_db.isOpen())
    return;

  QSqlQuery q(m_db);
  q.prepare("UPDATE sessions SET ended_at = ? WHERE session_id = ?");
  q.bindValue(0, QDateTime::currentDateTime().toString(Qt::ISODate));
  q.bindValue(1, m_sessionId);
  q.exec();
}

//--------------------------------------------------------------------------------------------------
// Export singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the SQLite export manager.
 */
Sessions::Export::Export()
  : DataModel::FrameConsumer<DataModel::TimestampedFramePtr>(
      DataModel::FrameConsumerConfig{8192, 1024, 1000})
  , m_isOpen(false)
  , m_exportEnabled(false)
  , m_rawBytesQueue(8192)
  , m_operationMode(static_cast<int>(AppState::instance().operationMode()))
{
  initializeWorker();
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
 * @brief Closes the current database file.
 */
void Sessions::Export::closeFile()
{
  if (m_isOpen.load(std::memory_order_relaxed)) {
    QMetaObject::invokeMethod(
      m_worker, &DataModel::FrameConsumerWorkerBase::close, Qt::BlockingQueuedConnection);

    m_isOpen.store(false, std::memory_order_relaxed);
    Q_EMIT openChanged();
  }
}

/**
 * @brief Wires external signals for auto-close on disconnect.
 */
void Sessions::Export::setupExternalConnections()
{
  // Mirror operation mode and force-off when entering Console-only
  connect(&AppState::instance(), &AppState::operationModeChanged, this, [this] {
    const auto mode = AppState::instance().operationMode();
    m_operationMode.store(static_cast<int>(mode), std::memory_order_relaxed);

    if (isOpen())
      closeFile();

    if (mode == SerialStudio::ConsoleOnly && exportEnabled())
      setExportEnabled(false);
  });

  // Close on disconnect
  connect(
    &IO::ConnectionManager::instance(), &IO::ConnectionManager::connectedChanged, this, [this] {
      if (!IO::ConnectionManager::instance().isConnected())
        closeFile();
    });

  // Close on player open (playback doesn't need SQLite export)
  connect(&CSV::Player::instance(), &CSV::Player::openChanged, this, [this] {
    if (CSV::Player::instance().isOpen())
      closeFile();
  });

  connect(&MDF4::Player::instance(), &MDF4::Player::openChanged, this, [this] {
    if (MDF4::Player::instance().isOpen())
      closeFile();
  });

  // Mirror ProjectModel into a thread-safe snapshot for the worker
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

  // Re-run on mode switch (ProjectModel's groups are not cleared)
  connect(&AppState::instance(),
          &AppState::operationModeChanged,
          this,
          &Sessions::Export::refreshProjectSnapshot);

  refreshProjectSnapshot();

  // Restore enabled state from settings (never enable in Console-only)
  const bool persisted = m_settings.value("SQLiteExport/Enabled", false).toBool();
  const bool allow = persisted && AppState::instance().operationMode() != SerialStudio::ConsoleOnly;
  m_exportEnabled.store(allow, std::memory_order_relaxed);
}

/**
 * @brief Main-thread-only: snapshots ProjectModel into m_projectSnapshot.
 */
void Sessions::Export::refreshProjectSnapshot()
{
  // Only snapshot when operation mode is set to project file
  QByteArray payload;
  if (AppState::instance().operationMode() == SerialStudio::ProjectFile) {
    const auto& pm = DataModel::ProjectModel::instance();
    if (!pm.groups().empty()) {
      const auto doc = QJsonDocument(pm.serializeToJson());
      payload        = doc.toJson(QJsonDocument::Compact);
    }
  }

  // Update the project snapshot
  QMutexLocker locker(&m_projectSnapshotMutex);
  m_projectSnapshot = std::move(payload);
}

/**
 * @brief Enables or disables SQLite export.
 */
void Sessions::Export::setExportEnabled(const bool enabled)
{
  // Refuse to enable while the app is in Console-only mode
  const bool allow = enabled && AppState::instance().operationMode() != SerialStudio::ConsoleOnly;

  // No-op when state is unchanged
  if (m_exportEnabled.load(std::memory_order_relaxed) == allow)
    return;

  // Close the open session when disabling
  if (!allow && isOpen())
    closeFile();

  // Apply and persist the new state
  m_exportEnabled.store(allow, std::memory_order_relaxed);
  setConsumerEnabled(allow);
  m_settings.setValue("SQLiteExport/Enabled", allow);
  Q_EMIT enabledChanged();
}

/**
 * @brief Enqueues a timestamped frame for SQLite export.
 */
void Sessions::Export::hotpathTxFrame(const DataModel::TimestampedFramePtr& frame)
{
  if (!m_exportEnabled.load(std::memory_order_relaxed))
    return;

  if (SerialStudio::isAnyPlayerOpen())
    return;

  enqueueData(frame);
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
                             &m_operationMode,
                             &m_projectSnapshotMutex,
                             &m_projectSnapshot);
  connect(w,
          &DataModel::FrameConsumerWorkerBase::resourceOpenChanged,
          this,
          &Export::onWorkerOpenChanged);
  return w;
}

/**
 * @brief Updates the isOpen state when the worker opens/closes the database.
 */
void Sessions::Export::onWorkerOpenChanged()
{
  auto* worker     = static_cast<ExportWorker*>(m_worker);
  const bool state = worker->isResourceOpen();
  if (m_isOpen.load(std::memory_order_relaxed) != state) {
    m_isOpen.store(state, std::memory_order_relaxed);
    Q_EMIT openChanged();
  }
}

#endif  // BUILD_COMMERCIAL

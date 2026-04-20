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

  m_dbOpen    = false;
  m_sessionId = -1;
  m_schema    = DataModel::ExportSchema{};
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

  // Honour the same enabled gate the base class uses — do not open a new
  // database or touch the raw queue when the consumer has been disabled.
  if (!consumerEnabled())
    return;

  // Console-only has no frames — open schema-less DB when raw bytes arrive.
  // Non-Console modes defer to processItems() for the proper template schema.
  if (!m_dbOpen
      && m_operationMode->load(std::memory_order_relaxed)
           == static_cast<int>(SerialStudio::ConsoleOnly)) {
    if (const auto* head = m_rawQueue->peek()) {
      DataModel::Frame emptyFrame;
      emptyFrame.title = QStringLiteral("ConsoleOnly");
      createDatabase(emptyFrame);
      if (m_dbOpen) {
        m_steadyBaseline = head->timestamp;
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

  // Create database on first batch — seed schema from the frame's own
  // groups/datasets; no cross-thread template handoff.
  if (!m_dbOpen) {
    createDatabase(items.front()->data);

    if (!m_dbOpen)
      return;

    m_steadyBaseline = items.front()->timestamp;
  }

  // Begin transaction for batch performance
  m_db.transaction();

  for (const auto& frame : items) {
    // Compute timestamp relative to session start
    const auto elapsed = frame->timestamp - m_steadyBaseline;
    const qint64 ns    = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();

    // Write readings for each dataset
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

  m_db.commit();
}

/**
 * @brief Creates the SQLite database file and writes the schema and session.
 */
void Sessions::ExportWorker::createDatabase(const DataModel::Frame& frame)
{
  Q_ASSERT(!m_dbOpen);

  // Use the canonical per-project path so all sessions for the same project
  // accumulate in a single .db file instead of one file per recording.
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

  // Insert the new session row. If this fails we abandon the open — proceeding
  // would let readings, raw_bytes, and columns rows reference session_id = -1
  // and poison the shared .db for every future browse/replay.
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
 * @brief Delegates to DatabaseManager::createSchema so Export and the
 *        read-side share a single source of truth for the session-log schema.
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
  // Serialize a replayable project JSON — falls back to synthesising from
  // the live frame when the user is recording in QuickPlot mode and
  // ProjectModel has no groups.
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
  // Same replayable snapshot the session row uses — ensures the global
  // project_metadata fallback matches what per-session rows reference.
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
 *
 * In ProjectFile mode the user's loaded project is the source of truth, so
 * we snapshot ProjectModel directly. In QuickPlot mode ProjectModel has no
 * groups (FrameBuilder synthesises them into m_quickPlotFrame), so we build
 * a minimal project JSON from the live frame itself — that's what the
 * replay Player reads back to restore the widget layout.
 */
QJsonObject Sessions::ExportWorker::buildReplayProjectJson(const DataModel::Frame& frame) const
{
  // Prefer the main-thread snapshot — ProjectModel is not thread-safe, so
  // touching its vectors from the worker would race with a user edit.
  {
    QMutexLocker locker(m_projectSnapshotMutex);
    if (!m_projectSnapshot->isEmpty()) {
      const auto doc = QJsonDocument::fromJson(*m_projectSnapshot);
      if (doc.isObject())
        return doc.object();
    }
  }

  // ConsoleOnly has no parsed structure — skip groups/actions/sources to
  // avoid leaking stale project data into the session row
  if (AppState::instance().operationMode() == SerialStudio::ConsoleOnly) {
    QJsonObject json;
    json.insert(QStringLiteral("title"), frame.title);
    json.insert(QStringLiteral("operationMode"), QStringLiteral("ConsoleOnly"));
    return json;
  }

  // QuickPlot / Console-only fallback — synthesise a minimal project from
  // the frame so Player::loadFromJsonDocument can restore widgets.
  QJsonObject json;
  json.insert(QStringLiteral("title"), frame.title);

  // Frame detection settings — QuickPlot uses line-based delimiters with
  // comma-separated values and PlainText decoding.
  json.insert(QStringLiteral("frameDetection"), static_cast<int>(SerialStudio::EndDelimiterOnly));
  json.insert(QStringLiteral("frameEnd"), QStringLiteral("\\n"));
  json.insert(QStringLiteral("frameStart"), QStringLiteral(""));
  json.insert(QStringLiteral("decoder"), static_cast<int>(SerialStudio::PlainText));

  // Groups from the live frame
  QJsonArray groupsArray;
  for (const auto& group : frame.groups)
    groupsArray.append(DataModel::serialize(group));

  json.insert(QStringLiteral("groups"), groupsArray);
  json.insert(QStringLiteral("actions"), QJsonArray());

  // Build a default source with a default template parser and matching
  // frame detection so FrameBuilder can parse the CSV-style data the
  // Player injects during replay. Prefer the Lua template; fall back to
  // JS if the Lua resource is somehow unavailable so the stored project
  // always carries a working (language, code) pair.
  int parserLanguage = static_cast<int>(SerialStudio::Lua);
  QString parserCode = DataModel::FrameParser::defaultTemplateCode(SerialStudio::Lua);
  if (parserCode.isEmpty()) {
    parserLanguage = static_cast<int>(SerialStudio::JavaScript);
    parserCode     = DataModel::FrameParser::defaultTemplateCode(SerialStudio::JavaScript);
  }

  QJsonObject source;
  source.insert(QStringLiteral("title"), QStringLiteral("Device A"));
  source.insert(QStringLiteral("sourceId"), 0);
  source.insert(QStringLiteral("busType"), static_cast<int>(SerialStudio::BusType::UART));
  source.insert(QStringLiteral("frameDetection"), static_cast<int>(SerialStudio::EndDelimiterOnly));
  source.insert(QStringLiteral("frameStart"), QString());
  source.insert(QStringLiteral("frameEnd"), QStringLiteral("\\n"));
  source.insert(QStringLiteral("decoder"), static_cast<int>(SerialStudio::PlainText));
  source.insert(QStringLiteral("frameParserLanguage"), parserLanguage);
  source.insert(QStringLiteral("frameParserCode"), parserCode);

  QJsonArray sourcesArray;
  sourcesArray.append(source);
  json.insert(QStringLiteral("sources"), sourcesArray);

  return json;
}

/**
 * @brief Prepares the pre-compiled queries used on the frame hotpath.
 */
void Sessions::ExportWorker::prepareHotpathQueries()
{
  m_readingQuery = QSqlQuery(m_db);
  m_readingQuery.prepare(
    "INSERT OR IGNORE INTO readings "
    "(session_id, timestamp_ns, unique_id, raw_numeric_value, raw_string_value, "
    " final_numeric_value, final_string_value, is_numeric) "
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

  m_rawBytesQuery = QSqlQuery(m_db);
  m_rawBytesQuery.prepare(
    "INSERT OR IGNORE INTO raw_bytes (session_id, timestamp_ns, device_id, data) "
    "VALUES (?, ?, ?, ?)");

  m_tableSnapshotQuery = QSqlQuery(m_db);
  m_tableSnapshotQuery.prepare(
    "INSERT OR IGNORE INTO table_snapshots "
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
    const auto elapsed = entry.timestamp - m_steadyBaseline;
    qint64 ns          = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();

    // Clamp: raw bytes may predate the first-frame baseline → negative ns
    // would collide on the (session, ts_ns, device) PK via INSERT OR IGNORE.
    if (ns < 0) [[unlikely]]
      ns = 0;

    m_rawBytesQuery.bindValue(0, m_sessionId);
    m_rawBytesQuery.bindValue(1, ns);
    m_rawBytesQuery.bindValue(2, entry.deviceId);
    m_rawBytesQuery.bindValue(3, entry.data ? *entry.data : QByteArray());

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
  // Mirror the main-thread operation mode into the worker-visible atomic so
  // the Console-only DB-open path in processData() knows when to trigger. If
  // the mode changes with an open session, close the current session — its
  // schema was chosen for the previous mode and continuing to write into it
  // would either orphan rows under the wrong title or mix incompatible data.
  connect(&AppState::instance(), &AppState::operationModeChanged, this, [this] {
    m_operationMode.store(static_cast<int>(AppState::instance().operationMode()),
                          std::memory_order_relaxed);

    if (isOpen())
      closeFile();
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

  // Mirror ProjectModel into a thread-safe snapshot the worker reads when
  // writing the replayable project_json column. Every signal that changes
  // the serialised shape of the project feeds this.
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
          &DataModel::ProjectModel::workspacesChanged,
          this,
          &Sessions::Export::refreshProjectSnapshot);

  // Switching operation modes (ProjectFile ↔ QuickPlot/ConsoleOnly) does NOT
  // clear ProjectModel's groups, so refreshProjectSnapshot must re-run to
  // avoid stamping QuickPlot sessions with a previously-loaded project.
  connect(&AppState::instance(),
          &AppState::operationModeChanged,
          this,
          &Sessions::Export::refreshProjectSnapshot);

  refreshProjectSnapshot();

  // Restore enabled state from settings
  m_exportEnabled.store(m_settings.value("SQLiteExport/Enabled", false).toBool(),
                        std::memory_order_relaxed);
}

/**
 * @brief Main-thread-only: snapshots ProjectModel into m_projectSnapshot so
 *        the worker thread never touches ProjectModel across threads.
 */
void Sessions::Export::refreshProjectSnapshot()
{
  QByteArray payload;

  // Only snapshot when ProjectFile mode is active — otherwise stale project
  // groups from a previous load would stamp QuickPlot/Console sessions.
  if (AppState::instance().operationMode() == SerialStudio::ProjectFile) {
    const auto& pm = DataModel::ProjectModel::instance();
    if (!pm.groups().empty()) {
      const auto doc = QJsonDocument(pm.serializeToJson());
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
  if (m_exportEnabled.load(std::memory_order_relaxed) == enabled)
    return;

  if (!enabled && isOpen())
    closeFile();

  m_exportEnabled.store(enabled, std::memory_order_relaxed);
  setConsumerEnabled(enabled);

  m_settings.setValue("SQLiteExport/Enabled", enabled);
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
void Sessions::Export::hotpathTxRawBytes(int deviceId, const IO::ByteArrayPtr& data)
{
  if (!m_exportEnabled.load(std::memory_order_relaxed))
    return;

  // No m_isOpen gate — Console-only opens the DB lazily from processData().
  TimestampedRawBytes entry;
  entry.deviceId  = deviceId;
  entry.data      = data;
  entry.timestamp = std::chrono::steady_clock::now();
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

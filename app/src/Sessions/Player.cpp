/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "Sessions/Player.h"

#  include <QApplication>
#  include <QDateTime>
#  include <QFileDialog>
#  include <QFileInfo>
#  include <QJsonDocument>
#  include <QJsonParseError>
#  include <QSqlError>
#  include <QSqlQuery>
#  include <QThread>
#  include <QTimer>
#  include <QtMath>

#  include "AppState.h"
#  include "DataModel/FrameBuilder.h"
#  include "DataModel/ProjectModel.h"
#  include "IO/ConnectionManager.h"
#  include "Misc/Utilities.h"
#  include "Misc/WorkspaceManager.h"
#  include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initializes member state, installs a global event filter, spins up the loader worker.
 */
Sessions::Player::Player()
  : m_workerThread(nullptr)
  , m_worker(nullptr)
  , m_frameQueryPrepared(false)
  , m_sessionId(-1)
  , m_pendingSessionId(-1)
  , m_loading(false)
  , m_framePos(0)
  , m_playing(false)
  , m_multiSource(false)
  , m_timestamp("--.--")
  , m_startTimestampSeconds(0.0)
  , m_preSessionCaptured(false)
  , m_preSessionOperationMode(SerialStudio::QuickPlot)
{
  qRegisterMetaType<Sessions::PlayerSessionPayloadPtr>("Sessions::PlayerSessionPayloadPtr");

  qApp->installEventFilter(this);
  connect(this, &Sessions::Player::playerStateChanged, this, &Sessions::Player::updateData);

  initWorker();
}

/**
 * @brief Closes the database before the singleton destructs.
 */
Sessions::Player::~Player()
{
  shutdown();
}

/**
 * @brief Returns the only instance of the class.
 */
Sessions::Player& Sessions::Player::instance()
{
  static Player singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Worker lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Spins up the loader worker on a dedicated thread and wires its signal.
 */
void Sessions::Player::initWorker()
{
  m_workerThread = new QThread;
  m_workerThread->setObjectName(QStringLiteral("Sessions::PlayerLoader"));

  m_worker = new PlayerLoaderWorker;
  m_worker->moveToThread(m_workerThread);

  connect(m_worker, &PlayerLoaderWorker::loaded, this, &Player::onLoadFinished);

  m_workerThread->start();
}

/**
 * @brief Closes the active database, cancels pending loads, joins the worker thread.
 */
void Sessions::Player::shutdown()
{
  closeFile();

  if (!m_workerThread)
    return;

  if (m_worker)
    m_worker->requestCancel();

  m_workerThread->quit();
  m_workerThread->wait(5000);

  // Worker thread has exited; safe to delete the worker directly from here.
  delete m_worker;
  m_worker = nullptr;

  delete m_workerThread;
  m_workerThread = nullptr;
}

//--------------------------------------------------------------------------------------------------
// Playback status queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns @c true while a session is loaded and ready to play.
 */
bool Sessions::Player::isOpen() const
{
  return m_db.isOpen() && m_sessionId >= 0 && !m_timestampsNs.empty();
}

/**
 * @brief Returns @c true while the worker is loading a session.
 */
bool Sessions::Player::loading() const
{
  return m_loading;
}

/**
 * @brief Returns @c true while frames are advancing on the live pipeline.
 */
bool Sessions::Player::isPlaying() const
{
  return m_playing;
}

/**
 * @brief Returns the number of frames in the loaded session.
 */
int Sessions::Player::frameCount() const
{
  return static_cast<int>(m_timestampsNs.size());
}

/**
 * @brief Returns the zero-based index of the current playback frame.
 */
int Sessions::Player::framePosition() const
{
  return m_framePos;
}

/**
 * @brief Returns a normalized playback position in the range [0, 1].
 */
double Sessions::Player::progress() const
{
  const auto count = frameCount();
  if (count <= 0)
    return 0.0;

  return static_cast<double>(framePosition()) / count;
}

/**
 * @brief Returns the short filename of the loaded database.
 */
QString Sessions::Player::filename() const
{
  if (!m_filePath.isEmpty())
    return QFileInfo(m_filePath).fileName();

  return QString();
}

/**
 * @brief Returns the formatted timestamp of the current frame.
 */
const QString& Sessions::Player::timestamp() const
{
  return m_timestamp;
}

//--------------------------------------------------------------------------------------------------
// Playback control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts real-time playback from the current frame.
 */
void Sessions::Player::play()
{
  if (!isOpen())
    return;

  if (m_framePos >= frameCount() - 1)
    m_framePos = 0;

  m_elapsedTimer.start();
  m_startTimestampSeconds = m_timestampsNs[static_cast<size_t>(m_framePos)] / 1e9;

  m_playing = true;
  Q_EMIT playerStateChanged();
}

/**
 * @brief Pauses playback without changing the current frame.
 */
void Sessions::Player::pause()
{
  if (!isOpen())
    return;

  m_playing = false;
  Q_EMIT playerStateChanged();
}

/**
 * @brief Toggles between play and pause.
 */
void Sessions::Player::toggle()
{
  if (m_playing)
    pause();
  else
    play();
}

//--------------------------------------------------------------------------------------------------
// File operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts the user to pick a .db file from the SQLite workspace folder.
 */
void Sessions::Player::openFile()
{
  auto* dialog = new QFileDialog(nullptr,
                                 tr("Open Session File"),
                                 Misc::WorkspaceManager::instance().path("Session Databases"),
                                 tr("Session files (*.db)"));

  dialog->setFileMode(QFileDialog::ExistingFile);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    if (!path.isEmpty())
      openFile(path);

    dialog->deleteLater();
  });

  connect(dialog, &QFileDialog::rejected, dialog, &QFileDialog::deleteLater);
  dialog->open();
}

/**
 * @brief Closes the active database, cancels any pending load, restores prior state.
 */
void Sessions::Player::closeFile()
{
  if (m_worker)
    m_worker->requestCancel();

  const bool wasLoading = m_loading;

  // Reset playback state
  m_playing  = false;
  m_framePos = 0;
  m_loading  = false;

  teardownLocalDb();
  clearLocalState();

  DataModel::FrameBuilder::instance().registerQuickPlotHeaders(QStringList());

  // Restore project + operation mode that existed before the session
  restorePreSessionState();

  if (wasLoading)
    Q_EMIT loadingChanged();

  Q_EMIT openChanged();
  Q_EMIT timestampChanged();
  Q_EMIT playerStateChanged();
}

/**
 * @brief Opens @p filePath, queues an async session load (latest session).
 */
void Sessions::Player::openFile(const QString& filePath)
{
  Q_ASSERT(!filePath.isEmpty());

  openFile(filePath, -1);
}

/**
 * @brief Opens a specific session from @p filePath via the loader worker.
 */
void Sessions::Player::openFile(const QString& filePath, int sessionId)
{
  Q_ASSERT(!filePath.isEmpty());

  if (filePath.isEmpty())
    return;

  // Cancel any in-flight load and tear down whatever is currently open
  closeFile();

  // Snapshot mode + project path before restoreProjectFromJson mutates them
  capturePreSessionState();

  // Prompt to disconnect live devices -- must happen on main thread
  if (IO::ConnectionManager::instance().isConnected()) {
    auto response =
      Misc::Utilities::showMessageBox(tr("Device Connection Active"),
                                      tr("To use this feature, you must disconnect from the "
                                         "device. Do you want to proceed?"),
                                      QMessageBox::Warning,
                                      qAppName(),
                                      QMessageBox::No | QMessageBox::Yes);

    if (response == QMessageBox::Yes)
      IO::ConnectionManager::instance().disconnectDevice();
    else
      return;
  }

  m_filePath         = filePath;
  m_pendingSessionId = sessionId;
  m_loading          = true;
  Q_EMIT loadingChanged();

  QMetaObject::invokeMethod(
    m_worker, "openAndLoad", Qt::QueuedConnection, Q_ARG(QString, filePath), Q_ARG(int, sessionId));
}

/**
 * @brief Worker shipped session bundle -- finalize setup on the main thread.
 */
void Sessions::Player::onLoadFinished(const PlayerSessionPayloadPtr& payload)
{
  // If the load was cancelled or superseded, the path may have been cleared
  if (!payload || m_filePath.isEmpty()) {
    if (m_loading) {
      m_loading = false;
      Q_EMIT loadingChanged();
    }

    return;
  }

  // Drop stale results from a load that was superseded by a fresh openFile()
  if (payload->filePath != m_filePath)
    return;

  if (m_pendingSessionId >= 0 && payload->sessionId != m_pendingSessionId) {
    if (m_loading) {
      m_loading = false;
      Q_EMIT loadingChanged();
    }

    return;
  }

  if (!payload->ok) {
    Misc::Utilities::showMessageBox(tr("Cannot open session file"),
                                    payload->error.isEmpty() ? tr("Unknown error") : payload->error,
                                    QMessageBox::Critical);

    m_loading = false;
    Q_EMIT loadingChanged();
    teardownLocalDb();
    clearLocalState();
    restorePreSessionState();
    Q_EMIT openChanged();
    Q_EMIT playerStateChanged();
    return;
  }

  // Restore project -- must happen on the main thread before column alignment
  if (!payload->projectJson.isEmpty()) {
    (void)restoreProjectFromJson(payload->projectJson);
  } else {
    // Inform the user the embedded project is missing
    Misc::Utilities::showMessageBox(tr("No project data"),
                                    tr("This session does not contain an embedded project file — "
                                       "the dashboard falls back to a quick-plot layout."),
                                    QMessageBox::Warning);
  }

  // Open the main-thread DB connection used for per-frame fetches
  if (!openLocalDb(m_filePath)) {
    m_loading = false;
    Q_EMIT loadingChanged();
    clearLocalState();
    restorePreSessionState();
    Q_EMIT openChanged();
    Q_EMIT playerStateChanged();
    return;
  }

  m_sessionId       = payload->sessionId;
  m_columnUniqueIds = payload->columnUniqueIds;
  m_timestampsNs    = payload->timestampsNs;

  const auto mode = AppState::instance().operationMode();
  if (mode == SerialStudio::ProjectFile) {
    alignColumnsToProject();

    const auto& sources = DataModel::ProjectModel::instance().sources();
    if (sources.size() > 1)
      buildMultiSourceMapping();
  } else {
    QStringList headers;
    headers.reserve(static_cast<int>(m_columnUniqueIds.size()));
    for (int uid : m_columnUniqueIds)
      headers.append(QStringLiteral("uid_%1").arg(uid));

    DataModel::FrameBuilder::instance().registerQuickPlotHeaders(headers);
  }

  if (m_uidToColumn.isEmpty())
    for (int i = 0; i < static_cast<int>(m_columnUniqueIds.size()); ++i)
      m_uidToColumn.insert(m_columnUniqueIds[static_cast<size_t>(i)], i);

  m_framePos              = 0;
  m_startTimestampSeconds = m_timestampsNs.front() / 1e9;

  m_loading = false;
  Q_EMIT loadingChanged();

  updateData();
  Q_EMIT openChanged();
  Q_EMIT playerStateChanged();
}

//--------------------------------------------------------------------------------------------------
// Local DB connection (per-frame fetch -- main thread)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens a main-thread @c QSqlDatabase connection used for per-frame queries.
 */
bool Sessions::Player::openLocalDb(const QString& filePath)
{
  m_connectionName = QStringLiteral("ss_sqlite_player_%1").arg(QDateTime::currentMSecsSinceEpoch());

  m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
  m_db.setDatabaseName(filePath);
  if (!m_db.open()) {
    Misc::Utilities::showMessageBox(tr("Cannot open session file"),
                                    tr("Check file permissions and try again."),
                                    QMessageBox::Critical);
    const QString conn = m_connectionName;
    m_db               = QSqlDatabase();
    if (!conn.isEmpty())
      QSqlDatabase::removeDatabase(conn);

    m_connectionName.clear();
    return false;
  }

  QSqlQuery pragma(m_db);
  pragma.exec("PRAGMA journal_mode=WAL");
  pragma.exec("PRAGMA busy_timeout=5000");
  return true;
}

/**
 * @brief Closes the per-frame DB connection and removes the named connection.
 */
void Sessions::Player::teardownLocalDb()
{
  m_frameQuery.clear();
  m_frameQuery         = QSqlQuery();
  m_frameQueryPrepared = false;

  if (m_db.isOpen())
    m_db.close();

  const QString conn = m_connectionName;
  m_db               = QSqlDatabase();
  if (!conn.isEmpty())
    QSqlDatabase::removeDatabase(conn);

  m_connectionName.clear();
}

/**
 * @brief Resets all per-session caches.
 */
void Sessions::Player::clearLocalState()
{
  m_filePath.clear();
  m_sessionId             = -1;
  m_pendingSessionId      = -1;
  m_timestamp             = "--.--";
  m_startTimestampSeconds = 0.0;
  m_multiSource           = false;
  m_columnUniqueIds.clear();
  m_uidToColumn.clear();
  m_timestampsNs.clear();
  m_columnToSource.clear();
  m_sourceColumnCount.clear();
}

//--------------------------------------------------------------------------------------------------
// Pre-session state capture / restore
//--------------------------------------------------------------------------------------------------

/**
 * @brief Snapshots the active operation mode and project file path.
 */
void Sessions::Player::capturePreSessionState()
{
  if (m_preSessionCaptured)
    return;

  m_preSessionOperationMode = AppState::instance().operationMode();
  m_preSessionProjectPath   = DataModel::ProjectModel::instance().jsonFilePath();
  m_preSessionCaptured      = true;
}

/**
 * @brief Restores the operation mode and project captured before the session.
 */
void Sessions::Player::restorePreSessionState()
{
  if (!m_preSessionCaptured)
    return;

  auto& pm = DataModel::ProjectModel::instance();
  if (!m_preSessionProjectPath.isEmpty() && QFileInfo::exists(m_preSessionProjectPath))
    (void)pm.openJsonFile(m_preSessionProjectPath);
  else
    pm.newJsonFile();

  AppState::instance().setOperationMode(m_preSessionOperationMode);

  m_preSessionCaptured = false;
  m_preSessionProjectPath.clear();
  m_preSessionOperationMode = SerialStudio::QuickPlot;
}

/**
 * @brief Loads the project JSON shipped by the worker.
 */
bool Sessions::Player::restoreProjectFromJson(const QString& json)
{
  if (json.isEmpty())
    return false;

  QJsonParseError parseError{};
  const auto doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);
  if (parseError.error != QJsonParseError::NoError || doc.isEmpty()) {
    qWarning() << "[Sessions::Player] Embedded project JSON is malformed:"
               << parseError.errorString();
    return false;
  }

  AppState::instance().setOperationMode(SerialStudio::ProjectFile);
  if (!DataModel::ProjectModel::instance().loadFromJsonDocument(doc)) {
    qWarning() << "[Sessions::Player] ProjectModel rejected the embedded JSON";
    return false;
  }

  return true;
}

//--------------------------------------------------------------------------------------------------
// Progress & seeking
//--------------------------------------------------------------------------------------------------

/**
 * @brief Seeks to a normalized position and rebuilds plot history around it.
 */
void Sessions::Player::setProgress(const double progress)
{
  if (!isOpen())
    return;

  const auto clamped = std::clamp(progress, 0.0, 1.0);
  if (isPlaying())
    pause();

  const int newPos = qMin(frameCount() - 1, qCeil(frameCount() * clamped));
  if (newPos == m_framePos)
    return;

  m_framePos = newPos;

  UI::Dashboard::instance().clearPlotData();

  const int toLoad   = UI::Dashboard::instance().points();
  const int startIdx = std::max(0, m_framePos - toLoad);
  processFrameBatch(startIdx, m_framePos);

  updateData();
}

/**
 * @brief Advances one frame and rebuilds the trailing plot window.
 */
void Sessions::Player::nextFrame()
{
  if (m_framePos < frameCount() - 1) {
    ++m_framePos;
    UI::Dashboard::instance().clearPlotData();
    const int toLoad   = UI::Dashboard::instance().points();
    const int startIdx = std::max(0, m_framePos - toLoad);
    processFrameBatch(startIdx, m_framePos);
    updateData();
  }
}

/**
 * @brief Steps one frame back and rebuilds the trailing plot window.
 */
void Sessions::Player::previousFrame()
{
  if (m_framePos > 0) {
    --m_framePos;
    UI::Dashboard::instance().clearPlotData();
    const int toLoad   = UI::Dashboard::instance().points();
    const int startIdx = std::max(0, m_framePos - toLoad);
    processFrameBatch(startIdx, m_framePos);
    updateData();
  }
}

//--------------------------------------------------------------------------------------------------
// Data processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Pushes the current frame and, if playing, schedules the next one.
 */
void Sessions::Player::updateData()
{
  if (!isOpen())
    return;

  updateTimestampDisplay();

  if (!isPlaying())
    return;

  injectFrame(buildFrameAt(m_timestampsNs[static_cast<size_t>(m_framePos)]));

  if (m_framePos >= frameCount() - 1) {
    pause();
    return;
  }

  // Schedule the next frame on the recording's wall-clock delta
  const qint64 elapsedMs = m_elapsedTimer.elapsed();
  const double nextSec   = m_timestampsNs[static_cast<size_t>(m_framePos + 1)] / 1e9;
  const double targetSec = m_startTimestampSeconds + (elapsedMs / 1000.0);
  qint64 msUntilNext     = qMax(0LL, static_cast<qint64>((nextSec - targetSec) * 1000.0));

  // Burst-emit late frames in one chunk to prevent runaway queues
  if (msUntilNext <= 0) {
    constexpr int kMaxBatchSize = 100;
    int processed               = 0;

    while (m_framePos < frameCount() - 1 && processed < kMaxBatchSize && msUntilNext <= 0) {
      ++m_framePos;
      injectFrame(buildFrameAt(m_timestampsNs[static_cast<size_t>(m_framePos)]));
      ++processed;

      if (m_framePos + 1 < frameCount()) {
        const double next   = m_timestampsNs[static_cast<size_t>(m_framePos + 1)] / 1e9;
        const double target = m_startTimestampSeconds + (m_elapsedTimer.elapsed() / 1000.0);
        msUntilNext         = qMax(0LL, static_cast<qint64>((next - target) * 1000.0));
      }

      else {
        pause();
        return;
      }
    }

    updateTimestampDisplay();

    if (m_framePos < frameCount() - 1)
      QTimer::singleShot(qMax(0LL, msUntilNext), Qt::PreciseTimer, this, [this] {
        if (isOpen() && isPlaying())
          updateData();
      });
    else
      pause();
  }

  else {
    QTimer::singleShot(msUntilNext, Qt::PreciseTimer, this, [this] {
      if (!isOpen() || !isPlaying())
        return;

      ++m_framePos;
      updateData();
    });
  }
}

/**
 * @brief Synchronously emits a contiguous frame range (used by seek/step).
 */
void Sessions::Player::processFrameBatch(int startFrame, int endFrame)
{
  if (!isOpen())
    return;

  Q_ASSERT(startFrame >= 0);
  Q_ASSERT(endFrame < frameCount());

  for (int i = startFrame; i <= endFrame; ++i)
    injectFrame(buildFrameAt(m_timestampsNs[static_cast<size_t>(i)]));
}

//--------------------------------------------------------------------------------------------------
// Column alignment / source mapping
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reorders @c m_columnUniqueIds to match FrameBuilder's parsing order.
 */
void Sessions::Player::alignColumnsToProject()
{
  if (m_columnUniqueIds.empty())
    return;

  // Recompute uniqueId locally (ProjectModel doesn't run finalize_frame)
  QMap<int, QPair<int, int>> uidToSrcIndex;
  const auto& groups = DataModel::ProjectModel::instance().groups();
  for (const auto& g : groups) {
    for (const auto& d : g.datasets) {
      const int uid = DataModel::dataset_unique_id(g, d);
      uidToSrcIndex.insert(uid, qMakePair(g.sourceId, d.index));
    }
  }

  // Bucket session columns by source so each source ends up index-sorted
  QMap<int, std::vector<QPair<int, int>>> bySource;
  std::vector<int> orphans;
  for (int uid : m_columnUniqueIds) {
    const auto it = uidToSrcIndex.constFind(uid);
    if (it == uidToSrcIndex.constEnd()) {
      orphans.push_back(uid);
      continue;
    }

    bySource[it.value().first].push_back(qMakePair(it.value().second, uid));
  }

  // Sort each source's slice by dataset.index ascending
  for (auto it = bySource.begin(); it != bySource.end(); ++it)
    std::sort(it.value().begin(), it.value().end(), [](const auto& a, const auto& b) {
      return a.first < b.first;
    });

  // Concatenate sources in ascending sourceId order, append unknown columns
  std::vector<int> aligned;
  aligned.reserve(m_columnUniqueIds.size());
  for (auto it = bySource.constBegin(); it != bySource.constEnd(); ++it)
    for (const auto& pair : it.value())
      aligned.push_back(pair.second);

  for (int uid : orphans)
    aligned.push_back(uid);

  m_columnUniqueIds.swap(aligned);

  m_uidToColumn.clear();
  for (int i = 0; i < static_cast<int>(m_columnUniqueIds.size()); ++i)
    m_uidToColumn.insert(m_columnUniqueIds[static_cast<size_t>(i)], i);
}

/**
 * @brief Builds the column->sourceId map; must run after alignColumnsToProject.
 */
void Sessions::Player::buildMultiSourceMapping()
{
  m_columnToSource.clear();
  m_sourceColumnCount.clear();
  m_uidToParser.clear();
  m_sourceMaxIndex.clear();

  // Recompute uniqueId locally and capture dataset.index per uid
  QMap<int, int> uidToSource;
  const auto& groups = DataModel::ProjectModel::instance().groups();
  for (const auto& g : groups) {
    for (const auto& d : g.datasets) {
      const int uid = DataModel::dataset_unique_id(g, d);
      uidToSource.insert(uid, g.sourceId);

      // Datasets sharing dataset.index collapse to the same channel slot
      if (d.index > 0) {
        m_uidToParser.insert(uid, qMakePair(g.sourceId, d.index));
        m_sourceMaxIndex[g.sourceId] = std::max(m_sourceMaxIndex.value(g.sourceId, 0), d.index);
      }
    }
  }

  for (int col = 0; col < static_cast<int>(m_columnUniqueIds.size()); ++col) {
    const int uid    = m_columnUniqueIds[static_cast<size_t>(col)];
    const auto srcIt = uidToSource.constFind(uid);
    if (srcIt == uidToSource.constEnd())
      continue;

    m_columnToSource[col] = srcIt.value();
    m_sourceColumnCount[srcIt.value()]++;
  }

  m_multiSource = !m_columnToSource.isEmpty() && m_sourceColumnCount.size() > 1;
}

//--------------------------------------------------------------------------------------------------
// Frame synthesis
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reconstructs a CSV-style payload for the given timestamp.
 */
QByteArray Sessions::Player::buildFrameAt(qint64 timestampNs)
{
  QStringList cells;
  cells.reserve(static_cast<int>(m_columnUniqueIds.size()));
  for (size_t i = 0; i < m_columnUniqueIds.size(); ++i)
    cells.append(QString());

  m_sourcesAtCurrentTs.clear();

  // Lazy-prepare the per-frame fetch query once and reuse it across ticks
  if (!m_frameQueryPrepared) {
    m_frameQuery = QSqlQuery(m_db);
    m_frameQuery.setForwardOnly(true);
    m_frameQuery.prepare("SELECT unique_id, raw_numeric_value, raw_string_value, is_numeric "
                         "FROM readings WHERE session_id = ? AND timestamp_ns = ? "
                         "ORDER BY reading_id");
    m_frameQueryPrepared = true;
  }

  m_frameQuery.bindValue(0, m_sessionId);
  m_frameQuery.bindValue(1, timestampNs);

  if (!m_frameQuery.exec()) [[unlikely]] {
    qWarning() << "[Sessions::Player] frame query failed:" << m_frameQuery.lastError().text();
    return QByteArray();
  }

  while (m_frameQuery.next()) {
    const int uid = m_frameQuery.value(0).toInt();
    const auto it = m_uidToColumn.constFind(uid);
    if (it == m_uidToColumn.constEnd())
      continue;

    const bool isNumeric = m_frameQuery.value(3).toInt() != 0;
    if (isNumeric) {
      const double v    = m_frameQuery.value(1).toDouble();
      cells[it.value()] = QString::number(v, 'g', 17);
    } else {
      cells[it.value()] = m_frameQuery.value(2).toString();
    }

    // Track which sources contributed data at this timestamp
    const auto srcIt = m_columnToSource.constFind(it.value());
    if (srcIt != m_columnToSource.constEnd())
      m_sourcesAtCurrentTs.insert(srcIt.value());
  }

  m_frameQuery.finish();

  QByteArray frame = cells.join(',').toUtf8();
  frame.append('\n');
  return frame;
}

/**
 * @brief Pushes @p frame into the live pipeline via ConnectionManager.
 */
void Sessions::Player::injectFrame(const QByteArray& frame)
{
  if (frame.isEmpty() || (frame.size() == 1 && frame.at(0) == '\n'))
    return;

  if (!m_multiSource) {
    IO::ConnectionManager::instance().processPayload(frame);
    return;
  }

  // Multi-source: synthesize per-source channel arrays via column -> (sourceId, parserIndex)
  const auto fields = QString::fromUtf8(frame).trimmed().split(',');

  // Allocate per-source channel arrays with maxIndex slots.
  QHash<int, QStringList> sourceChannels;
  for (auto it = m_sourceMaxIndex.constBegin(); it != m_sourceMaxIndex.constEnd(); ++it)
    if (m_sourcesAtCurrentTs.contains(it.key()))
      sourceChannels[it.key()] = QStringList(it.value(), QString());

  for (int col = 0; col < fields.size() && col < static_cast<int>(m_columnUniqueIds.size());
       ++col) {
    const int uid  = m_columnUniqueIds[static_cast<size_t>(col)];
    const auto pIt = m_uidToParser.constFind(uid);
    if (pIt == m_uidToParser.constEnd())
      continue;

    const int srcId     = pIt.value().first;
    const int parserIdx = pIt.value().second;
    if (!m_sourcesAtCurrentTs.contains(srcId))
      continue;

    auto cit = sourceChannels.find(srcId);
    if (cit == sourceChannels.end())
      continue;

    auto& slotList = cit.value();
    if (parserIdx >= 1 && parserIdx <= slotList.size())
      slotList[parserIdx - 1] = fields[col];
  }

  if (sourceChannels.isEmpty())
    return;

  QMap<int, QByteArray> sourcePayloads;
  for (auto it = sourceChannels.constBegin(); it != sourceChannels.constEnd(); ++it)
    sourcePayloads[it.key()] = it.value().join(',').toUtf8() + '\n';

  IO::ConnectionManager::instance().processMultiSourcePayload(frame, sourcePayloads);
}

//--------------------------------------------------------------------------------------------------
// Display helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates @c m_timestamp to HH:MM:SS.mmm of the current frame's offset.
 */
void Sessions::Player::updateTimestampDisplay()
{
  if (m_framePos < 0 || m_framePos >= frameCount())
    return;

  const double seconds = m_timestampsNs[static_cast<size_t>(m_framePos)] / 1e9;
  m_timestamp          = formatTimestamp(seconds);
  Q_EMIT timestampChanged();
}

/**
 * @brief Formats @p seconds as HH:MM:SS.mmm for the player status label.
 */
QString Sessions::Player::formatTimestamp(double seconds) const
{
  int hours   = static_cast<int>(seconds / 3600.0);
  int minutes = static_cast<int>((seconds - hours * 3600.0) / 60.0);
  double secs = seconds - hours * 3600.0 - minutes * 60.0;

  return QString("%1:%2:%3")
    .arg(hours, 2, 10, QChar('0'))
    .arg(minutes, 2, 10, QChar('0'))
    .arg(secs, 6, 'f', 3, QChar('0'));
}

//--------------------------------------------------------------------------------------------------
// Event handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Intercepts global key events while a session is loaded.
 */
bool Sessions::Player::eventFilter(QObject* obj, QEvent* event)
{
  if (isOpen() && event->type() == QEvent::KeyPress) {
    auto* keyEvent = static_cast<QKeyEvent*>(event);
    return handleKeyPress(keyEvent);
  }

  return QObject::eventFilter(obj, event);
}

/**
 * @brief Maps Space / arrow / media keys to play/pause and frame stepping.
 */
bool Sessions::Player::handleKeyPress(QKeyEvent* keyEvent)
{
  bool handled = false;
  switch (keyEvent->key()) {
    case Qt::Key_Space:
    case Qt::Key_MediaPlay:
    case Qt::Key_MediaPause:
    case Qt::Key_MediaTogglePlayPause:
      toggle();
      handled = true;
      break;
    case Qt::Key_Left:
    case Qt::Key_Down:
    case Qt::Key_MediaPrevious:
      previousFrame();
      handled = true;
      break;
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_MediaNext:
      nextFrame();
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

#endif  // BUILD_COMMERCIAL

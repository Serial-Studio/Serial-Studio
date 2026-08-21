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

#  include "Sessions/Player.h"

#  include <algorithm>
#  include <bit>
#  include <cmath>
#  include <cstring>
#  include <limits>
#  include <QApplication>
#  include <QDateTime>
#  include <QDeadlineTimer>
#  include <QFile>
#  include <QFileDialog>
#  include <QFileInfo>
#  include <QJsonDocument>
#  include <QJsonParseError>
#  include <QScopedValueRollback>
#  include <QSqlError>
#  include <QSqlQuery>
#  include <QtEndian>
#  include <QThread>
#  include <QTimer>
#  include <QtMath>
#  include <unordered_map>

#  include "AppState.h"
#  include "DataModel/FrameBuilder.h"
#  include "DataModel/NotificationCenter.h"
#  include "DataModel/ProjectModel.h"
#  include "DataModel/Scripting/FrameParserPipeline.h"
#  include "IO/ConnectionManager.h"
#  include "Misc/Utilities.h"
#  include "Misc/WorkspaceManager.h"
#  include "Sessions/BlockReader.h"
#  include "Sessions/StreamBlockCodec.h"
#  include "SSAssert.h"
#  include "UI/Dashboard.h"

static constexpr int kSessionMaxSeekWindowRows = 262144;

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
  , m_seekQueryPrepared(false)
  , m_hasFinalValues(false)
  , m_usesBlocks(false)
  , m_sessionId(-1)
  , m_pendingSessionId(-1)
  , m_loading(false)
  , m_framePos(0)
  , m_playing(false)
  , m_multiSource(false)
  , m_injecting(false)
  , m_timestamp("--.--")
  , m_startTimestampSeconds(0.0)
  , m_steadyBaseRowSeconds(0.0)
  , m_preSessionCaptured(false)
  , m_restorePending(false)
  , m_preSessionOperationMode(SerialStudio::QuickPlot)
{
  qRegisterMetaType<Sessions::PlayerSessionPayloadPtr>("Sessions::PlayerSessionPayloadPtr");

  qApp->installEventFilter(this);
  connect(this, &Sessions::Player::playerStateChanged, this, &Sessions::Player::updateData);

  constexpr int kSeekTickMs   = 33;
  constexpr int kSeekSettleMs = 250;
  m_seekTimer.setSingleShot(true);
  m_seekTimer.setInterval(kSeekTickMs);
  m_settleTimer.setSingleShot(true);
  m_settleTimer.setInterval(kSeekSettleMs);
  connect(&m_seekTimer, &QTimer::timeout, this, &Sessions::Player::performSeekTick);
  connect(&m_settleTimer, &QTimer::timeout, this, &Sessions::Player::performSeekSettle);

  initWorker();
}

/**
 * @brief Joins the loader worker only: the dtor runs at static destruction, after
 *        SessionContext::shutdown() has freed FrameBuilder and the other core modules, so the
 *        closeFile() half of shutdown() must never run here (2026-07 Windows teardown AV).
 */
Sessions::Player::~Player()
{
  joinWorker();
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
 * @brief Closes the active database, cancels pending loads, joins the worker thread. Call while
 *        the core modules are still alive (ModuleManager::onQuit); closeFile() touches them.
 */
void Sessions::Player::shutdown()
{
  closeFile();
  joinWorker();
}

/**
 * @brief Cancels and joins the loader worker; touches only Player-owned state.
 */
void Sessions::Player::joinWorker()
{
  if (!m_workerThread)
    return;

  if (m_worker)
    m_worker->requestCancel();

  m_workerThread->quit();
  const bool joined = m_workerThread->wait(5000);

  if (!joined) {
    qWarning() << "[Sessions::Player] Loader thread did not stop within 5 s; leaking the "
                  "worker and thread to avoid tearing down objects it still uses";
    m_worker       = nullptr;
    m_workerThread = nullptr;
    return;
  }

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
  return m_db && m_db->isOpen() && m_sessionId >= 0 && !m_timestampsNs.empty();
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

  m_seekTimer.stop();
  m_settleTimer.stop();

  anchorSteadyBase(m_framePos);
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
  static auto& workspaceManager = Misc::WorkspaceManager::instance();
  auto* dialog                  = new QFileDialog(qApp->activeWindow(),
                                 tr("Open Session File"),
                                 workspaceManager.path("Session Databases"),
                                 tr("Session files (*.db)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(this, [this, path]() { openFile(path); }, Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Closes the active database, cancels any pending load, restores prior state.
 */
void Sessions::Player::closeFile()
{
  // code-verify off
  // Never tear the session down from inside an inject: the builder is still reading the cells
  // this call staged. Re-queue; the inject returns within one marshal.
  // code-verify on
  if (m_injecting) {
    QMetaObject::invokeMethod(this, [this] { closeFile(); }, Qt::QueuedConnection);
    return;
  }

  if (m_worker)
    m_worker->requestCancel();

  const bool wasLoading = m_loading;

  m_playing  = false;
  m_framePos = 0;
  m_loading  = false;
  m_seekTimer.stop();
  m_settleTimer.stop();

  teardownLocalDb();
  clearLocalState();

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  frameBuilder.registerQuickPlotHeaders(QStringList());
  frameBuilder.setReplayColumnMap({});

  schedulePreSessionRestore();

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
  SS_ASSERT_LOG(!filePath.isEmpty());

  openFile(filePath, -1);
}

/**
 * @brief Opens a specific session from @p filePath via the loader worker.
 */
void Sessions::Player::openFile(const QString& filePath, int sessionId)
{
  if (filePath.isEmpty())
    return;

  closeFile();

  m_restorePending = false;
  capturePreSessionState();

  static auto& connectionManager = IO::ConnectionManager::instance();
  if (connectionManager.isConnected()) {
    auto response =
      Misc::Utilities::showMessageBox(tr("Device Connection Active"),
                                      tr("To use this feature, you must disconnect from the "
                                         "device. Do you want to proceed?"),
                                      QMessageBox::Warning,
                                      qAppName(),
                                      QMessageBox::No | QMessageBox::Yes);

    if (response == QMessageBox::Yes)
      connectionManager.disconnectDevice();
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
  if (!payload || m_filePath.isEmpty()) {
    if (m_loading) {
      m_loading = false;
      Q_EMIT loadingChanged();
    }

    return;
  }

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
    schedulePreSessionRestore();
    Q_EMIT openChanged();
    Q_EMIT playerStateChanged();
    return;
  }

  if (!payload->projectJson.isEmpty()) {
    (void)restoreProjectFromJson(payload->projectJson);
    applyBundledViewState(payload->viewState, payload->projectJson);
  } else {
    Misc::Utilities::showMessageBox(tr("No project data"),
                                    tr("This session does not contain an embedded project file — "
                                       "the dashboard falls back to a quick-plot layout."),
                                    QMessageBox::Warning);
  }

  // code-verify off
  // The id must be set BEFORE openLocalDb: detectFinalValueColumns probes
  // "blocks WHERE session_id = m_sessionId", and the stale -1 from the previous close made a
  // block-format session read the empty readings table for the whole replay (spec 0064).
  // code-verify on
  m_sessionId = payload->sessionId;

  if (!openLocalDb(m_filePath)) {
    m_loading = false;
    Q_EMIT loadingChanged();
    clearLocalState();
    schedulePreSessionRestore();
    Q_EMIT openChanged();
    Q_EMIT playerStateChanged();
    return;
  }

  m_columnUniqueIds = payload->columnUniqueIds;
  m_timestampsNs    = payload->timestampsNs;
  m_streamBlocks    = payload->streamBlocks;
  mergeStreamBlockTimes();

  static auto& appState = AppState::instance();
  const auto mode       = appState.operationMode();
  if (mode == SerialStudio::ProjectFile) {
    alignColumnsToProject();
    buildMultiSourceMapping();
  } else {
    QStringList headers;
    headers.reserve(static_cast<int>(m_columnUniqueIds.size()));
    for (int uid : m_columnUniqueIds)
      headers.append(QStringLiteral("uid_%1").arg(uid));

    static auto& frameBuilder = DataModel::FrameBuilder::instance();
    frameBuilder.registerQuickPlotHeaders(headers);
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
// Local DB connection (per-frame fetch on main thread)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens a main-thread @c QSqlDatabase connection used for per-frame queries.
 */
bool Sessions::Player::openLocalDb(const QString& filePath)
{
  m_connectionName = QStringLiteral("ss_sqlite_player_%1").arg(QDateTime::currentMSecsSinceEpoch());

  m_db.emplace(QSqlDatabase::addDatabase("QSQLITE", m_connectionName));
  m_db->setDatabaseName(filePath);
  if (!m_db->open()) {
    Misc::Utilities::showMessageBox(tr("Cannot open session file"),
                                    tr("Check file permissions and try again."),
                                    QMessageBox::Critical);
    const QString conn = m_connectionName;
    m_db.reset();
    if (!conn.isEmpty())
      QSqlDatabase::removeDatabase(conn);

    m_connectionName.clear();
    return false;
  }

  QSqlQuery pragma(*m_db);
  pragma.exec("PRAGMA journal_mode=WAL");
  pragma.exec("PRAGMA busy_timeout=5000");

  detectFinalValueColumns();
  return true;
}

/**
 * @brief Probes the readings schema for the final-value columns (absent in old session files).
 */
void Sessions::Player::detectFinalValueColumns()
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

/**
 * @brief Closes the per-frame DB connection and removes the named connection.
 */
void Sessions::Player::teardownLocalDb()
{
  if (m_frameQuery)
    m_frameQuery->clear();

  if (m_seekQuery)
    m_seekQuery->clear();

  m_frameQuery.reset();
  m_seekQuery.reset();
  m_streamBlobQuery.reset();
  m_frameQueryPrepared = false;
  m_seekQueryPrepared  = false;

  if (m_db && m_db->isOpen())
    m_db->close();

  const QString conn = m_connectionName;
  m_db.reset();
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
  m_hasFinalValues        = false;
  m_usesBlocks            = false;
  m_columnUniqueIds       = {};
  m_uidToColumn.clear();
  m_timestampsNs = {};
  m_columnToSource.clear();
  m_sourceColumns.clear();
  m_streamBlocks     = {};
  m_streamChannelBuf = {};
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

  static auto& appState     = AppState::instance();
  static auto& projectModel = DataModel::ProjectModel::instance();
  m_preSessionOperationMode = appState.operationMode();
  m_preSessionProjectPath   = projectModel.jsonFilePath();
  m_preSessionViewState     = viewStateDashboard().viewStateJson();
  m_preSessionCaptured      = true;
}

/**
 * @brief The dashboard the view-state bundle (spec 0062) is read from and applied to.
 */
UI::Dashboard& Sessions::Player::viewStateDashboard()
{
  static auto& dashboard = UI::Dashboard::instance();
  return dashboard;
}

/**
 * @brief Applies a recording's view-state bundle after its project was restored (widgets read it
 *        in Component.onCompleted, so ordering falls out of the rebuild) and warns once when the
 *        embedded project differs from the one loaded before playback (R5, notice only: the
 *        recording's project always wins, as playback did before this spec).
 */
void Sessions::Player::applyBundledViewState(const QString& viewState, const QString& projectJson)
{
  auto& dashboard = viewStateDashboard();
  if (viewState.isEmpty())
    dashboard.clearViewState();
  else
    dashboard.setViewStateJson(viewState);

  if (projectJson.isEmpty() || m_preSessionProjectPath.isEmpty())
    return;

  QFile file(m_preSessionProjectPath);
  if (!file.open(QIODevice::ReadOnly))
    return;

  const auto live     = QJsonDocument::fromJson(file.readAll());
  const auto embedded = QJsonDocument::fromJson(projectJson.toUtf8());
  if (!live.isObject() || !embedded.isObject() || live.object() == embedded.object())
    return;

  static auto& notifications = DataModel::NotificationCenter::instance();
  notifications.postWarning(
    tr("Sessions"),
    tr("Recording uses an older copy of the project"),
    tr("The dashboard shown is the one embedded in the recording; the project on disk has "
       "changed since. Close the session to return to the current project."));
}

/**
 * @brief Queues the pre-session restore instead of running it on the caller's stack: closing a
 *        session can be driven from a QML handler firing during window destruction, and the
 *        restore reloads the project, which rebuilds devices and waits on the frame pipeline.
 *        Inline, that re-enters the platform event loop against a half-torn-down window.
 */
void Sessions::Player::schedulePreSessionRestore()
{
  if (!m_preSessionCaptured)
    return;

  m_restorePending = true;
  QMetaObject::invokeMethod(this, [this] { performPendingRestore(); }, Qt::QueuedConnection);
}

/**
 * @brief Runs a queued restore unless it was cancelled by a session opened in the meantime.
 */
void Sessions::Player::performPendingRestore()
{
  if (!m_restorePending)
    return;

  m_restorePending = false;
  restorePreSessionState();
}

/**
 * @brief Restores the operation mode and project captured before the session.
 */
void Sessions::Player::restorePreSessionState()
{
  if (!m_preSessionCaptured)
    return;

  static auto& pm = DataModel::ProjectModel::instance();
  if (!m_preSessionProjectPath.isEmpty() && QFileInfo::exists(m_preSessionProjectPath))
    (void)pm.openJsonFile(m_preSessionProjectPath);
  else
    pm.newJsonFile();

  static auto& appState = AppState::instance();
  appState.setOperationMode(m_preSessionOperationMode);

  viewStateDashboard().setViewStateJson(m_preSessionViewState);

  m_preSessionCaptured = false;
  m_preSessionProjectPath.clear();
  m_preSessionViewState.clear();
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

  static auto& appState = AppState::instance();
  appState.setOperationMode(SerialStudio::ProjectFile);

  static auto& projectModel = DataModel::ProjectModel::instance();
  if (!projectModel.loadFromJsonDocument(doc)) {
    qWarning() << "[Sessions::Player] ProjectModel rejected the embedded JSON";
    return false;
  }

  return true;
}

//--------------------------------------------------------------------------------------------------
// Progress & seeking
//--------------------------------------------------------------------------------------------------

/**
 * @brief Seeks to a normalized position (tape scrub, spec 0020): the position and timestamp
 *        update immediately, a coalescing timer live-fills the plots at ~30 Hz, and the settle
 *        timer runs the exact full-window rebuild once the slider rests.
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
  updateTimestampDisplay();

  if (!m_seekTimer.isActive())
    m_seekTimer.start();

  m_settleTimer.start();
}

/**
 * @brief First row of the scrub window ending at @p target: walks back until the plot time
 *        range is covered (never fewer than points() rows), capped at kSessionMaxSeekWindowRows so
 *        dense recordings bound the per-tick cost.
 */
int Sessions::Player::seekWindowStartRow(int target) const
{
  SS_ASSERT(target >= 0, return 0);
  SS_ASSERT(target < static_cast<int>(m_timestampsNs.size()),
            return qMax(0, static_cast<int>(m_timestampsNs.size()) - 1));

  static auto& dashboard = UI::Dashboard::instance();
  const double range     = dashboard.plotTimeRange();
  const double targetSec = m_timestampsNs[static_cast<size_t>(target)] / 1e9;

  const int minStart = qMax(0, target - qMax(1, dashboard.points()) + 1);
  const int capStart = qMax(0, target - kSessionMaxSeekWindowRows + 1);

  int start = minStart;
  for (int i = 0; i < kSessionMaxSeekWindowRows && start > capStart; ++i) {
    const double sec = m_timestampsNs[static_cast<size_t>(start - 1)] / 1e9;
    if (targetSec - sec > range)
      break;

    --start;
  }

  return start;
}

/**
 * @brief One coalesced scrub tick: bulk-fills the plot rings from the trailing window ending
 *        at the cursor and injects the cursor row so scalar widgets track it. Outside
 *        ProjectFile mode the settle rebuild runs instead -- a bulk fill with an empty
 *        series map would wipe the rings and blank the plots for the whole drag.
 */
void Sessions::Player::performSeekTick()
{
  if (!isOpen() || isPlaying())
    return;

  SS_ASSERT(m_framePos >= 0, return);
  SS_ASSERT(m_framePos < frameCount(), return);

  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile || m_uidToColumn.isEmpty()) {
    performSeekSettle();
    return;
  }

  static auto& dashboard = UI::Dashboard::instance();
  const int target       = m_framePos;
  const int start        = seekWindowStartRow(target);

  QVector<double> times;
  QHash<qint64, QVector<double>> series;
  buildSeekWindow(start, target, times, series);
  dashboard.bulkLoadPlotWindow(times, series);

  anchorSteadyBase(target);
  injectFrame(buildFrameAt(m_timestampsNs[static_cast<size_t>(target)]),
              m_timestampsNs[static_cast<size_t>(target)]);
}

/**
 * @brief At-rest settle pass: exact trailing-window replay through the fast lane (FFT and the
 *        other frame-fed widgets), then a full-time-window bulk fill so the plots keep the
 *        complete tape view instead of collapsing to the pipeline batch.
 */
void Sessions::Player::performSeekSettle()
{
  if (!isOpen() || isPlaying())
    return;

  SS_ASSERT(m_framePos >= 0, return);
  SS_ASSERT(m_framePos < frameCount(), return);

  static auto& dashboard = UI::Dashboard::instance();
  dashboard.clearPlotData();

  const int window = qMin(dashboard.points(), m_framePos + 1);
  const int start  = qMax(0, m_framePos - window + 1);
  processFrameBatch(start, m_framePos);

  static auto& appState = AppState::instance();
  if (appState.operationMode() == SerialStudio::ProjectFile && !m_uidToColumn.isEmpty()) {
    QVector<double> times;
    QHash<qint64, QVector<double>> series;
    const int fillStart = seekWindowStartRow(m_framePos);
    buildSeekWindow(fillStart, m_framePos, times, series);
    dashboard.bulkLoadPlotWindow(times, series);
  }

  updateData();
}

/**
 * @brief Forward-fills NaN gaps and backfills the leading run from the first stored value.
 */
static void fillSessionSeekGaps(QVector<double>& values)
{
  int firstSet = -1;
  const int n  = values.size();
  for (int k = 0; k < n; ++k)
    if (std::isnan(values[k]))
      values[k] = (k > 0) ? values[k - 1] : values[k];
    else if (firstSet < 0)
      firstSet = k;

  const double seed = (firstSet >= 0) ? values[firstSet] : 0.0;
  for (int k = 0; k < n && std::isnan(values[k]); ++k)
    values[k] = seed;
}

/**
 * @brief Fills the seek-window times and per-(source, uid) numeric series with one windowed
 *        range query over readings (covering index; ties broken by reading_id). Sparse
 *        readings forward-fill, and leading gaps backfill from the first stored value.
 */
void Sessions::Player::buildSeekWindow(int startRow,
                                       int endRow,
                                       QVector<double>& times,
                                       QHash<qint64, QVector<double>>& series)
{
  SS_ASSERT(startRow >= 0, return);
  SS_ASSERT(startRow <= endRow, return);
  SS_ASSERT(endRow < frameCount(), return);

  const int n = endRow - startRow + 1;
  times.resize(n);
  for (int k = 0; k < n; ++k)
    times[k] = m_timestampsNs[static_cast<size_t>(startRow + k)] / 1e9;

  if (!m_db) [[unlikely]]
    return;

  static auto& dashboard = UI::Dashboard::instance();
  const auto pairs       = dashboard.replaySeekSeries();
  QHash<int, qint64> keyByUid;
  for (const auto& pair : pairs) {
    const auto colIt = m_uidToColumn.constFind(pair.second);
    if (colIt == m_uidToColumn.constEnd())
      continue;

    const qint64 key = UI::Dashboard::replaySeekKey(pair.first, pair.second);
    keyByUid.insert(pair.second, key);
    series.insert(key, QVector<double>(n, std::numeric_limits<double>::quiet_NaN()));
  }

  if (keyByUid.isEmpty())
    return;

  if (m_usesBlocks) {
    fillSeekWindowFromBlocks(startRow, endRow, keyByUid, series);
    return;
  }

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
      qWarning() << "[Sessions::Player] seek query prepare failed:"
                 << m_seekQuery->lastError().text();
      m_seekQuery.reset();
      series.clear();
      return;
    }

    m_seekQueryPrepared = true;
  }

  m_seekQuery->bindValue(0, m_sessionId);
  m_seekQuery->bindValue(1, m_timestampsNs[static_cast<size_t>(startRow)]);
  m_seekQuery->bindValue(2, m_timestampsNs[static_cast<size_t>(endRow)]);

  if (!m_seekQuery->exec()) [[unlikely]] {
    qWarning() << "[Sessions::Player] seek window query failed:" << m_seekQuery->lastError().text();
    series.clear();
    return;
  }

  const auto begin = m_timestampsNs.cbegin() + startRow;
  const auto end   = m_timestampsNs.cbegin() + endRow + 1;
  while (m_seekQuery->next()) {
    const auto keyIt = keyByUid.constFind(m_seekQuery->value(0).toInt());
    if (keyIt == keyByUid.constEnd())
      continue;

    const qint64 tsNs = m_seekQuery->value(2).toLongLong();
    const auto pos    = std::lower_bound(begin, end, tsNs);
    if (pos == end || *pos != tsNs)
      continue;

    const int idx              = static_cast<int>(pos - begin);
    series[keyIt.value()][idx] = SerialStudio::toDouble(m_seekQuery->value(1));
  }

  m_seekQuery->finish();

  for (auto it = series.begin(); it != series.end(); ++it)
    fillSessionSeekGaps(it.value());
}

/**
 * @brief Advances one frame and rebuilds the trailing plot window.
 */
void Sessions::Player::nextFrame()
{
  if (m_framePos < frameCount() - 1) {
    ++m_framePos;
    static auto& dashboard = UI::Dashboard::instance();
    dashboard.clearPlotData();
    const int toLoad   = dashboard.points();
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
    static auto& dashboard = UI::Dashboard::instance();
    dashboard.clearPlotData();
    const int toLoad   = dashboard.points();
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

  injectFrame(buildFrameAt(m_timestampsNs[static_cast<size_t>(m_framePos)]),
              m_timestampsNs[static_cast<size_t>(m_framePos)]);

  if (m_framePos >= frameCount() - 1) {
    pause();
    return;
  }

  constexpr double kInvMs = 1.0 / 1000.0;
  constexpr double kInvNs = 1.0 / 1e9;
  const qint64 elapsedMs  = m_elapsedTimer.elapsed();
  const double nextSec    = m_timestampsNs[static_cast<size_t>(m_framePos + 1)] * kInvNs;
  const double targetSec  = m_startTimestampSeconds + (elapsedMs * kInvMs);
  qint64 msUntilNext      = qMax(0LL, static_cast<qint64>((nextSec - targetSec) * 1000.0));

  if (msUntilNext <= 0) {
    constexpr qint64 kCatchUpBudgetMs = 20;
    constexpr int kCatchUpMaxFrames   = 4096;
    const QDeadlineTimer budget(kCatchUpBudgetMs);

    int processed = 0;
    while (m_framePos < frameCount() - 1 && msUntilNext <= 0 && !budget.hasExpired()
           && processed < kCatchUpMaxFrames) {
      ++m_framePos;
      ++processed;
      injectFrame(buildFrameAt(m_timestampsNs[static_cast<size_t>(m_framePos)]),
                  m_timestampsNs[static_cast<size_t>(m_framePos)]);

      if (m_framePos + 1 < frameCount()) {
        const double next   = m_timestampsNs[static_cast<size_t>(m_framePos + 1)] * kInvNs;
        const double target = m_startTimestampSeconds + (m_elapsedTimer.elapsed() * kInvMs);
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
        if (isOpen() && isPlaying()) {
          ++m_framePos;
          updateData();
        }
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

  SS_ASSERT(startFrame >= 0, return);
  SS_ASSERT(endFrame < frameCount(), return);

  anchorSteadyBase(startFrame);
  for (int i = startFrame; i <= endFrame; ++i)
    injectFrame(buildFrameAt(m_timestampsNs[static_cast<size_t>(i)]),
                m_timestampsNs[static_cast<size_t>(i)]);
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

  QMap<int, QPair<int, int>> uidToSrcIndex;
  static auto& projectModel = DataModel::ProjectModel::instance();
  const auto& groups        = projectModel.groups();
  for (const auto& g : groups)
    for (const auto& d : g.datasets)
      uidToSrcIndex.insert(d.uniqueId, qMakePair(g.sourceId, d.index));

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

  for (auto it = bySource.begin(); it != bySource.end(); ++it)
    std::sort(it.value().begin(), it.value().end(), [](const auto& a, const auto& b) {
      return a.first < b.first;
    });

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
 * @brief Builds the per-source column lists and installs the FrameBuilder replay map
 *        (uid -> payload cell index); runs for any source count. Single-source payloads
 *        travel through processPayload, which routes to source 0, so the map is rekeyed.
 */
void Sessions::Player::buildMultiSourceMapping()
{
  m_columnToSource.clear();
  m_sourceColumns.clear();

  QMap<int, int> uidToSource;
  static auto& projectModel = DataModel::ProjectModel::instance();
  const auto& groups        = projectModel.groups();
  for (const auto& g : groups)
    for (const auto& d : g.datasets)
      uidToSource.insert(d.uniqueId, g.sourceId);

  std::unordered_map<int, std::unordered_map<int, int>> replay;
  for (int col = 0; col < static_cast<int>(m_columnUniqueIds.size()); ++col) {
    const int uid    = m_columnUniqueIds[static_cast<size_t>(col)];
    const auto srcIt = uidToSource.constFind(uid);
    if (srcIt == uidToSource.constEnd())
      continue;

    const int srcId    = srcIt.value();
    auto& columns      = m_sourceColumns[srcId];
    replay[srcId][uid] = static_cast<int>(columns.size());
    columns.push_back(uid);

    m_columnToSource[col] = srcId;
  }

  m_multiSource = m_sourceColumns.size() > 1;

  if (!m_multiSource && !replay.empty() && replay.begin()->first != 0) {
    auto columns = std::move(replay.begin()->second);
    replay.clear();
    replay[0] = std::move(columns);
  }

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  frameBuilder.setReplayColumnMap(std::move(replay));
}

//--------------------------------------------------------------------------------------------------
// Frame synthesis
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the readings row for @p timestampNs into a uid -> cell text map. Replays the
 *        stored final (post-transform) values; raw columns are the fallback for old files.
 */
QHash<int, QString> Sessions::Player::buildFrameAt(qint64 timestampNs)
{
  QHash<int, QString> uidValues;
  uidValues.reserve(static_cast<int>(m_columnUniqueIds.size()));

  m_sourcesAtCurrentTs.clear();

  if (!m_db) [[unlikely]]
    return uidValues;

  if (m_usesBlocks)
    return frameValuesFromBlocks(timestampNs);

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
    qWarning() << "[Sessions::Player] frame query failed:" << m_frameQuery->lastError().text();
    return uidValues;
  }

  while (m_frameQuery->next()) {
    const int uid = m_frameQuery->value(0).toInt();
    const auto it = m_uidToColumn.constFind(uid);
    if (it == m_uidToColumn.constEnd())
      continue;

    const bool isNumeric = m_frameQuery->value(3).toInt() != 0;
    if (isNumeric) {
      const double v = SerialStudio::toDouble(m_frameQuery->value(1));
      uidValues[uid] = QString::number(v, 'g', 17);
    } else {
      uidValues[uid] = m_frameQuery->value(2).toString();
    }

    const auto srcIt = m_columnToSource.constFind(it.value());
    if (srcIt != m_columnToSource.constEnd())
      m_sourcesAtCurrentTs.insert(srcIt.value());
  }

  m_frameQuery->finish();
  return uidValues;
}

/**
 * @brief Spec-0055 twin of the seek window: selects the blocks overlapping the window by their
 *        indexed [t0_ns, t_end_ns] span, decodes them, and drops each sample onto its row. The
 *        span index is what keeps this a lookup rather than a decode of the whole session.
 */
void Sessions::Player::fillSeekWindowFromBlocks(int startRow,
                                                int endRow,
                                                const QHash<int, qint64>& keyByUid,
                                                QHash<qint64, QVector<double>>& series)
{
  const qint64 fromNs = m_timestampsNs[static_cast<size_t>(startRow)];
  const qint64 toNs   = m_timestampsNs[static_cast<size_t>(endRow)];

  QSqlQuery q(*m_db);
  q.setForwardOnly(true);
  q.prepare(QStringLiteral("SELECT %1 FROM blocks WHERE session_id = ? AND t_end_ns >= ? "
                           "AND t0_ns <= ? ORDER BY t0_ns, block_id")
              .arg(QLatin1String(Sessions::kBlockColumns)));
  q.bindValue(0, m_sessionId);
  q.bindValue(1, fromNs);
  q.bindValue(2, toNs);

  if (!q.exec()) [[unlikely]] {
    qWarning() << "[Sessions::Player] block seek query failed:" << q.lastError().text();
    series.clear();
    return;
  }

  const auto begin = m_timestampsNs.cbegin() + startRow;
  const auto end   = m_timestampsNs.cbegin() + endRow + 1;

  std::vector<Sessions::ReadingRow> rows;
  while (q.next()) {
    rows.clear();
    if (!Sessions::decodeBlockRow(q, rows))
      continue;

    for (const auto& row : rows) {
      const auto keyIt = keyByUid.constFind(row.uniqueId);
      if (keyIt == keyByUid.constEnd())
        continue;

      const auto pos = std::lower_bound(begin, end, row.timestampNs);
      if (pos == end || *pos != row.timestampNs)
        continue;

      series[keyIt.value()][static_cast<int>(pos - begin)] = row.finalNumeric;
    }
  }

  q.finish();

  for (auto it = series.begin(); it != series.end(); ++it)
    fillSessionSeekGaps(it.value());
}

/**
 * @brief Spec-0055 twin of the cursor-row read: the blocks containing @p timestampNs are those
 *        whose indexed span covers it, and the sample at that exact instant is the replayed cell.
 */
QHash<int, QString> Sessions::Player::frameValuesFromBlocks(qint64 timestampNs)
{
  QHash<int, QString> uidValues;

  QSqlQuery q(*m_db);
  q.setForwardOnly(true);
  q.prepare(QStringLiteral("SELECT %1 FROM blocks WHERE session_id = ? AND t0_ns <= ? "
                           "AND t_end_ns >= ? ORDER BY block_id")
              .arg(QLatin1String(Sessions::kBlockColumns)));
  q.bindValue(0, m_sessionId);
  q.bindValue(1, timestampNs);
  q.bindValue(2, timestampNs);

  if (!q.exec()) [[unlikely]] {
    qWarning() << "[Sessions::Player] block frame query failed:" << q.lastError().text();
    return uidValues;
  }

  std::vector<Sessions::ReadingRow> rows;
  while (q.next()) {
    rows.clear();
    if (!Sessions::decodeBlockRow(q, rows))
      continue;

    for (const auto& row : rows) {
      if (row.timestampNs != timestampNs)
        continue;

      const auto it = m_uidToColumn.constFind(row.uniqueId);
      if (it == m_uidToColumn.constEnd())
        continue;

      uidValues[row.uniqueId] =
        row.isNumeric ? QString::number(row.finalNumeric, 'g', 17) : row.finalString;

      const auto srcIt = m_columnToSource.constFind(it.value());
      if (srcIt != m_columnToSource.constEnd())
        m_sourcesAtCurrentTs.insert(srcIt.value());
    }
  }

  q.finish();
  return uidValues;
}

/**
 * @brief Anchors the steady-clock base used to stamp replayed rows with recorded deltas.
 */
void Sessions::Player::anchorSteadyBase(int frameIndex)
{
  SS_ASSERT(frameIndex >= 0, frameIndex = 0);

  m_steadyBase = std::chrono::steady_clock::now();
  m_steadyBaseRowSeconds =
    (frameIndex < frameCount()) ? m_timestampsNs[static_cast<size_t>(frameIndex)] / 1e9 : 0.0;
}

/**
 * @brief Steady timestamp for @p timestampNs: the anchored base advanced by the recorded
 *        delta, so the recording -- not the wall clock -- owns replay time.
 */
std::chrono::steady_clock::time_point Sessions::Player::rowSteadyTimestamp(qint64 timestampNs) const
{
  const auto delta = std::chrono::duration<double>(timestampNs / 1e9 - m_steadyBaseRowSeconds);
  return m_steadyBase + std::chrono::duration_cast<std::chrono::steady_clock::duration>(delta);
}

/**
 * @brief Feeds per-source cell lists in stored column order through the FrameBuilder replay
 *        fast lane (spec 0020) with the recorded timestamp; the single-source map is rekeyed
 *        to source 0, matching buildMultiSourceMapping. QuickPlot mode keeps the byte path
 *        (its parser consumes raw payloads).
 */
void Sessions::Player::injectFrame(const QHash<int, QString>& uidValues, qint64 timestampNs)
{
  injectStreamBlocksAt(timestampNs);

  if (uidValues.isEmpty())
    return;

  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile) {
    QStringList cells;
    cells.reserve(static_cast<int>(m_columnUniqueIds.size()));
    for (int uid : m_columnUniqueIds)
      cells.append(uidValues.value(uid));

    QByteArray payload = DataModel::joinReplayRow(cells);
    payload.append('\n');
    static auto& connectionManager = IO::ConnectionManager::instance();
    connectionManager.processPayload(payload);
    return;
  }

  if (m_sourcesAtCurrentTs.isEmpty())
    return;

  // code-verify off
  // replayChannels() marshals blocking and pumps this thread's event loop, so a queued close can
  // clear these members mid-loop. Guard against re-entry and walk copies, as the other players do.
  // code-verify on
  if (m_injecting)
    return;

  const QScopedValueRollback<bool> reentry_guard(m_injecting, true);

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  const auto timestamp      = rowSteadyTimestamp(timestampNs);
  const auto sources        = m_sourcesAtCurrentTs;
  const auto sourceColumns  = m_sourceColumns;

  for (int srcId : std::as_const(sources)) {
    const auto colsIt = sourceColumns.constFind(srcId);
    if (colsIt == sourceColumns.constEnd() || colsIt.value().empty())
      continue;

    QStringList cells;
    cells.reserve(static_cast<int>(colsIt.value().size()));
    for (int uid : colsIt.value())
      cells.append(uidValues.value(uid));

    frameBuilder.replayChannels(m_multiSource ? srcId : 0, cells, timestamp);
  }
}

//--------------------------------------------------------------------------------------------------
// Stream-block replay (spec 0054)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Folds each block's start time into the playback clock, so a session whose data is
 *        entirely stream-lane still advances: the player steps over block starts (block rate),
 *        never over individual samples, which is what keeps the index bounded.
 */
void Sessions::Player::mergeStreamBlockTimes()
{
  if (m_streamBlocks.empty())
    return;

  m_timestampsNs.reserve(m_timestampsNs.size() + m_streamBlocks.size());
  for (const auto& entry : m_streamBlocks)
    m_timestampsNs.push_back(entry.t0Ns);

  std::sort(m_timestampsNs.begin(), m_timestampsNs.end());
  m_timestampsNs.erase(std::unique(m_timestampsNs.begin(), m_timestampsNs.end()),
                       m_timestampsNs.end());
}

/**
 * @brief Reads one block's samples blob by rowid and decodes it from canonical little-endian
 *        float64. Rejects a blob whose length is not `frames * 8` rather than decoding past its
 *        end -- a truncated or foreign file must fail loudly, not silently misplay.
 */
bool Sessions::Player::fetchStreamSamples(qint64 rowId, qint64 frames, std::vector<double>& out)
{
  SS_ASSERT(frames >= 0, return false);

  if (!m_db || !m_db->isOpen()) [[unlikely]]
    return false;

  if (!m_streamBlobQuery) {
    m_streamBlobQuery.emplace(*m_db);
    m_streamBlobQuery->setForwardOnly(true);
    m_streamBlobQuery->prepare("SELECT samples FROM stream_blocks WHERE stream_block_id = ?");
  }

  m_streamBlobQuery->bindValue(0, rowId);
  if (!m_streamBlobQuery->exec() || !m_streamBlobQuery->next()) [[unlikely]] {
    qWarning() << "[Sessions::Player] stream block fetch failed:"
               << m_streamBlobQuery->lastError().text();
    return false;
  }

  const QByteArray blob = m_streamBlobQuery->value(0).toByteArray();
  m_streamBlobQuery->finish();

  if (!unpackStreamSamples(blob, frames, out)) [[unlikely]] {
    qWarning() << "[Sessions::Player] stream block" << rowId << "has" << blob.size()
               << "bytes, expected" << (frames * kStreamSampleBytes);
    return false;
  }

  return true;
}

/**
 * @brief Replays one source's slice of a block: decodes each channel straight into a DataBlock
 *        and publishes it through the same tail a live source uses (spec 0055). Only this block's
 *        channels are resident. QuickPlot has no project groups, so the session's own column order
 *        stands in for the empty source-column map.
 */
void Sessions::Player::replayStreamGroup(int sourceId, std::size_t first, std::size_t last)
{
  SS_ASSERT(first < m_streamBlocks.size(), return);
  SS_ASSERT(last <= m_streamBlocks.size(), return);

  const auto colsIt               = m_sourceColumns.constFind(sourceId);
  const bool mapped               = colsIt != m_sourceColumns.constEnd() && !colsIt.value().empty();
  const std::vector<int>& columns = mapped ? colsIt.value() : m_columnUniqueIds;
  if (columns.empty())
    return;

  m_streamChannelBuf.resize(columns.size());

  QHash<int, std::size_t> uidToSlot;
  for (std::size_t c = 0; c < columns.size(); ++c)
    uidToSlot.insert(columns[c], c);

  qint64 frames = 0;
  qint64 t0Ns   = m_streamBlocks[first].t0Ns;
  qint64 dtNs   = m_streamBlocks[first].dtNs;
  for (std::size_t b = first; b < last; ++b) {
    const auto& entry = m_streamBlocks[b];
    const auto slotIt = uidToSlot.constFind(entry.uniqueId);
    if (slotIt == uidToSlot.constEnd())
      continue;

    if (fetchStreamSamples(entry.rowId, entry.frames, m_streamChannelBuf[slotIt.value()]))
      frames = std::max(frames, entry.frames);
  }

  if (frames <= 0)
    return;

  auto block                 = std::make_shared<DataModel::DataBlock>();
  block->sourceId            = sourceId;
  block->structureGeneration = 0;
  block->samples             = frames;
  block->t0                  = rowSteadyTimestamp(t0Ns);
  block->dt                  = std::chrono::nanoseconds(dtNs > 0 ? dtNs : 1);

  block->columns.resize(columns.size());
  for (std::size_t c = 0; c < columns.size(); ++c) {
    auto& column    = block->columns[c];
    column.uniqueId = columns[c];
    column.hasText  = false;
    column.hasRaw   = false;
    column.values.assign(static_cast<std::size_t>(frames), 0.0);

    const auto& samples = m_streamChannelBuf[c];
    const auto used     = std::min(samples.size(), static_cast<std::size_t>(frames));
    std::copy_n(samples.begin(), used, column.values.begin());
  }

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  frameBuilder.replayBlock(block);
}

/**
 * @brief Replays every stream block whose start time is @p timestampNs. Blocks arrive as a
 *        burst with per-sample timestamps, exactly as they do live -- the recording, not the
 *        wall clock, owns the sample times.
 */
void Sessions::Player::injectStreamBlocksAt(qint64 timestampNs)
{
  if (m_streamBlocks.empty())
    return;

  const auto begin = std::lower_bound(
    m_streamBlocks.begin(),
    m_streamBlocks.end(),
    timestampNs,
    [](const PlayerStreamBlockIndex& entry, qint64 ts) { return entry.t0Ns < ts; });

  std::size_t i          = static_cast<std::size_t>(begin - m_streamBlocks.begin());
  const std::size_t size = m_streamBlocks.size();
  while (i < size && m_streamBlocks[i].t0Ns == timestampNs) {
    const int sourceId   = m_streamBlocks[i].sourceId;
    std::size_t groupEnd = i;
    while (groupEnd < size && m_streamBlocks[groupEnd].t0Ns == timestampNs
           && m_streamBlocks[groupEnd].sourceId == sourceId)
      ++groupEnd;

    replayStreamGroup(sourceId, i, groupEnd);
    i = groupEnd;
  }
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
  constexpr double kInvHour = 1.0 / 3600.0;
  constexpr double kInvMin  = 1.0 / 60.0;
  int hours                 = static_cast<int>(seconds * kInvHour);
  int minutes               = static_cast<int>((seconds - hours * 3600.0) * kInvMin);
  double secs               = seconds - hours * 3600.0 - minutes * 60.0;

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

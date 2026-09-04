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
#  include <limits>
#  include <QApplication>
#  include <QDeadlineTimer>
#  include <QFile>
#  include <QFileDialog>
#  include <QFileInfo>
#  include <QGuiApplication>
#  include <QJsonDocument>
#  include <QJsonParseError>
#  include <QThread>
#  include <QTimer>
#  include <QtMath>
#  include <span>

#  include "AppState.h"
#  include "DataModel/FrameBuilder.h"
#  include "DataModel/NotificationCenter.h"
#  include "DataModel/ProjectModel.h"
#  include "IO/ConnectionManager.h"
#  include "Misc/Utilities.h"
#  include "Misc/WorkspaceManager.h"
#  include "Sessions/Player/ReplayAlignment.h"
#  include "SSAssert.h"
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
  , m_loading(false)
  , m_playing(false)
  , m_framePos(0)
  , m_pendingSessionId(-1)
  , m_timestamp("--.--")
  , m_startTimestampSeconds(0.0)
  , m_layout()
  , m_restorePending(false)
{
  qRegisterMetaType<Sessions::PlayerSessionPayloadPtr>("Sessions::PlayerSessionPayloadPtr");

  qApp->installEventFilter(this);
  connect(this, &Sessions::Player::playerStateChanged, this, &Sessions::Player::updateData);

  connect(&m_engine,
          &DataModel::ReplayPlaybackEngine::seekTick,
          this,
          &Sessions::Player::performSeekTick);
  connect(&m_engine,
          &DataModel::ReplayPlaybackEngine::seekSettle,
          this,
          &Sessions::Player::performSeekSettle);

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

/**
 * @brief Builds the frame synthesis on first use and returns it. Deferred rather than built in the
 *        constructor because the pipeline objects it takes are singletons the composition root is
 *        still ordering when the player itself is created; first use is always a playback action,
 *        long after every one of them exists.
 */
Sessions::ReplaySynthesis& Sessions::Player::synthesis()
{
  if (!m_synthesis) {
    static auto& appState          = AppState::instance();
    static auto& frameBuilder      = DataModel::FrameBuilder::instance();
    static auto& connectionManager = IO::ConnectionManager::instance();

    m_synthesis = std::make_unique<ReplaySynthesis>(
      m_reader, m_layout, appState, frameBuilder, connectionManager);
  }

  return *m_synthesis;
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
  return m_reader.isOpen() && m_reader.sessionId() >= 0 && !m_timestampsNs.empty();
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

  (void)m_engine.nextEpoch();
  m_elapsedTimer.start();
  m_startTimestampSeconds = m_timestampsNs[static_cast<size_t>(m_framePos)] / 1e9;
  m_engine.stopSeek();

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

  (void)m_engine.nextEpoch();
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
  if (m_synthesis && m_synthesis->injecting()) {
    QMetaObject::invokeMethod(this, [this] { closeFile(); }, Qt::QueuedConnection);
    return;
  }

  if (m_worker)
    m_worker->requestCancel();

  const bool wasLoading = m_loading;

  m_playing  = false;
  m_framePos = 0;
  m_loading  = false;
  m_engine.stopSeek();

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

  if (!m_reader.open(m_filePath, payload->sessionId)) {
    Misc::Utilities::showMessageBox(tr("Cannot open session file"),
                                    tr("Check file permissions and try again."),
                                    QMessageBox::Critical);

    m_loading = false;
    Q_EMIT loadingChanged();
    clearLocalState();
    schedulePreSessionRestore();
    Q_EMIT openChanged();
    Q_EMIT playerStateChanged();
    return;
  }

  m_layout.columnUniqueIds = payload->columnUniqueIds;
  m_timestampsNs           = payload->timestampsNs;
  synthesis().setStreamBlocks(payload->streamBlocks);
  ReplayAlignment::mergeStreamBlockTimes(m_timestampsNs, payload->streamBlocks);

  static auto& appState = AppState::instance();
  if (appState.operationMode() == SerialStudio::ProjectFile)
    applyProjectLayout();
  else
    registerQuickPlotColumns();

  if (m_layout.uidToColumn.isEmpty())
    ReplayAlignment::indexColumns(m_layout);

  m_framePos              = 0;
  m_startTimestampSeconds = m_timestampsNs.front() / 1e9;

  m_loading = false;
  Q_EMIT loadingChanged();

  updateData();
  Q_EMIT openChanged();
  Q_EMIT playerStateChanged();
}

/**
 * @brief Resets all per-session state: the read connection, the column layout, the timeline and
 *        the synthesis caches.
 */
void Sessions::Player::clearLocalState()
{
  m_reader.close();
  if (m_synthesis)
    m_synthesis->clear();

  m_filePath.clear();
  m_pendingSessionId      = -1;
  m_timestamp             = "--.--";
  m_startTimestampSeconds = 0.0;
  m_timestampsNs          = {};
  m_layout                = ReplayLayout();
}

//--------------------------------------------------------------------------------------------------
// Column alignment / source mapping
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers synthetic headers for a recording replayed outside ProjectFile mode: QuickPlot
 *        has no project to align against, so the stored column order names the series.
 */
void Sessions::Player::registerQuickPlotColumns()
{
  QStringList headers;
  headers.reserve(static_cast<int>(m_layout.columnUniqueIds.size()));
  for (int uid : m_layout.columnUniqueIds)
    headers.append(QStringLiteral("uid_%1").arg(uid));

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  frameBuilder.registerQuickPlotHeaders(headers);
}

/**
 * @brief Reconciles the recording's stored columns with the loaded project: the parsing order, the
 *        per-source column lists and the FrameBuilder replay map. The project is flattened to a
 *        location map here so the alignment arithmetic itself never reads ProjectModel.
 */
void Sessions::Player::applyProjectLayout()
{
  DatasetLocationMap locations;
  static auto& projectModel = DataModel::ProjectModel::instance();
  const auto& groups        = projectModel.groups();
  for (const auto& g : groups)
    for (const auto& d : g.datasets)
      locations.insert(d.uniqueId, DatasetLocation{g.sourceId, d.index});

  ReplayAlignment::alignColumnsToProject(m_layout, locations);
  auto replayMap = ReplayAlignment::buildMultiSourceMapping(m_layout, locations);

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  frameBuilder.setReplayColumnMap(std::move(replayMap));
}

//--------------------------------------------------------------------------------------------------
// Pre-session state capture / restore
//--------------------------------------------------------------------------------------------------

/**
 * @brief Snapshots the active operation mode and project file path.
 */
void Sessions::Player::capturePreSessionState()
{
  if (m_preSession.captured())
    return;

  static auto& appState     = AppState::instance();
  static auto& projectModel = DataModel::ProjectModel::instance();
  m_preSession.capture(
    appState.operationMode(), projectModel.jsonFilePath(), viewStateDashboard().viewStateJson());
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

  const auto& capturedProject = m_preSession.projectPath();
  if (projectJson.isEmpty() || capturedProject.isEmpty())
    return;

  QFile file(capturedProject);
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
  if (!m_preSession.captured())
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
  if (!m_preSession.captured())
    return;

  static auto& pm         = DataModel::ProjectModel::instance();
  const auto& projectPath = m_preSession.projectPath();
  if (!projectPath.isEmpty() && QFileInfo::exists(projectPath))
    (void)pm.openJsonFile(projectPath);
  else
    pm.newJsonFile();

  static auto& appState = AppState::instance();
  appState.setOperationMode(m_preSession.operationMode());

  viewStateDashboard().setViewStateJson(m_preSession.viewState());

  m_preSession.clear();
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

  m_engine.armSeek();
}

/**
 * @brief First row of the scrub window ending at @p target: walks back until the plot time
 *        range is covered (never fewer than points() rows), capped by the engine so
 *        dense recordings bound the per-tick cost.
 */
int Sessions::Player::seekWindowStartRow(int target) const
{
  SS_ASSERT(target >= 0, return 0);
  SS_ASSERT(target < static_cast<int>(m_timestampsNs.size()),
            return qMax(0, static_cast<int>(m_timestampsNs.size()) - 1));

  static auto& dashboard = UI::Dashboard::instance();
  return DataModel::ReplayPlaybackEngine::seekWindowStartRow(
    target, dashboard.points(), dashboard.plotTimeRange(), [this](int row) {
      return m_timestampsNs[static_cast<size_t>(row)] / 1e9;
    });
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
  if (appState.operationMode() != SerialStudio::ProjectFile || m_layout.uidToColumn.isEmpty()) {
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
  synthesis().replayFrameAt(m_timestampsNs[static_cast<size_t>(target)]);
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
  if (appState.operationMode() == SerialStudio::ProjectFile && !m_layout.uidToColumn.isEmpty()) {
    QVector<double> times;
    QHash<qint64, QVector<double>> series;
    const int fillStart = seekWindowStartRow(m_framePos);
    buildSeekWindow(fillStart, m_framePos, times, series);
    dashboard.bulkLoadPlotWindow(times, series);
  }

  updateData();
}

/**
 * @brief Fills the seek-window times and per-(source, uid) numeric series for the rows in
 *        [@p startRow, @p endRow]. The dashboard names the series it wants, the reader fills them
 *        from whichever table the recording uses.
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

  if (!m_reader.isOpen()) [[unlikely]]
    return;

  static auto& dashboard = UI::Dashboard::instance();
  const auto pairs       = dashboard.replaySeekSeries();
  QHash<int, qint64> keyByUid;
  for (const auto& pair : pairs) {
    const auto colIt = m_layout.uidToColumn.constFind(pair.second);
    if (colIt == m_layout.uidToColumn.constEnd())
      continue;

    const qint64 key = UI::Dashboard::replaySeekKey(pair.first, pair.second);
    keyByUid.insert(pair.second, key);
    series.insert(key, QVector<double>(n, std::numeric_limits<double>::quiet_NaN()));
  }

  if (keyByUid.isEmpty())
    return;

  const std::span<const qint64> rowTimes(m_timestampsNs.data() + startRow,
                                         static_cast<std::size_t>(n));
  m_reader.fillSeekWindow(rowTimes, keyByUid, series);
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

  synthesis().replayFrameAt(m_timestampsNs[static_cast<size_t>(m_framePos)]);

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
    constexpr int kCatchUpMaxFrames = 4096;
    const QDeadlineTimer budget(DataModel::ReplayPlaybackEngine::kCatchUpBudgetMs);

    int processed = 0;
    while (m_framePos < frameCount() - 1 && msUntilNext <= 0 && !budget.hasExpired()
           && processed < kCatchUpMaxFrames) {
      ++m_framePos;
      ++processed;
      synthesis().replayFrameAt(m_timestampsNs[static_cast<size_t>(m_framePos)]);

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

    if (m_framePos < frameCount() - 1) {
      const quint64 epoch = m_engine.epoch();
      QTimer::singleShot(qMax(0LL, msUntilNext), Qt::PreciseTimer, this, [this, epoch] {
        if (isOpen() && isPlaying() && m_engine.isCurrentEpoch(epoch)) {
          ++m_framePos;
          updateData();
        }
      });
    } else
      pause();
  }

  else {
    const quint64 epoch = m_engine.epoch();
    QTimer::singleShot(msUntilNext, Qt::PreciseTimer, this, [this, epoch] {
      if (!isOpen() || !isPlaying() || !m_engine.isCurrentEpoch(epoch))
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
    synthesis().replayFrameAt(m_timestampsNs[static_cast<size_t>(i)]);
}

/**
 * @brief Anchors the replay clock at frame @p frameIndex, so the rows that follow are stamped with
 *        their recorded deltas from it.
 */
void Sessions::Player::anchorSteadyBase(int frameIndex)
{
  SS_ASSERT(frameIndex >= 0, frameIndex = 0);

  const double rowSeconds =
    (frameIndex < frameCount()) ? m_timestampsNs[static_cast<size_t>(frameIndex)] / 1e9 : 0.0;
  synthesis().anchorSteadyBase(rowSeconds);
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
  m_timestamp          = DataModel::ReplayPlaybackEngine::formatTimestamp(seconds);
  Q_EMIT timestampChanged();
}

//--------------------------------------------------------------------------------------------------
// Event handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Intercepts global key events while a session is loaded.
 */

/**
 * @brief Captures key events and routes playback shortcuts to handleKeyPress.
 */
bool Sessions::Player::eventFilter(QObject* obj, QEvent* event)
{
  if (isOpen() && event->type() == QEvent::KeyPress) {
    auto* keyEvent = static_cast<QKeyEvent*>(event);
    if (!DataModel::ReplayPlaybackEngine::playbackKeyIsClaimed(keyEvent->key()))
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

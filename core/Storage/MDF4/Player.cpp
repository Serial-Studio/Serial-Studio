/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "Player.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <QApplication>
#include <QDeadlineTimer>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QScopedValueRollback>
#include <QTimer>
#include <unordered_map>

#include "AppState.h"
#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/IO/IPayloadInjector.h"
#include "Core/Prompt/UserPrompt.h"
#include "Core/Services.h"
#include "Core/SSAssert.h"
#include "Core/WorkspaceManager.h"
#include "DataModel/IReplayPlotSink.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/FrameParserPipeline.h"
#include "Replay/LinkGate.h"

#ifdef BUILD_COMMERCIAL
#  include "Core/Licensing/CommercialToken.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructor - Initializes the MDF4 player
 */
MDF4::Player::Player()
  : m_framePos(0)
  , m_injecting(false)
  , m_playing(false)
  , m_open(false)
  , m_decoding(false)
  , m_multiSource(false)
  , m_decodeProgress(0.0)
  , m_timestamp("")
  , m_startTimestamp(0.0)
  , m_decodeGeneration(0)
  , m_loaderThread(nullptr)
  , m_loader(nullptr)
  , m_bus(nullptr)
  , m_plotSink(nullptr)
  , m_payloadInjector(nullptr)
{
  qApp->installEventFilter(this);
  qRegisterMetaType<MDF4::PlayerDecodePayloadPtr>();
  connect(this, &MDF4::Player::playerStateChanged, this, &MDF4::Player::updateData);

  connect(
    &m_engine, &DataModel::ReplayPlaybackEngine::seekTick, this, &MDF4::Player::performSeekTick);
  connect(&m_engine,
          &DataModel::ReplayPlaybackEngine::seekSettle,
          this,
          &MDF4::Player::performSeekSettle);
}

/**
 * @brief Destructor - joins any in-flight decode so the worker never outlives the player.
 */
MDF4::Player::~Player()
{
  stopDecoding();
}

/**
 * @brief Returns the singleton instance of the MDF4 player
 */
MDF4::Player& MDF4::Player::instance()
{
  static Player singleton;
  return singleton;
}

/**
 * @brief Adopts the root-owned message bus this module publishes on and subscribes to.
 */
void MDF4::Player::attachMessageBus(Core::Bus::MessageBus& bus)
{
  SS_ASSERT(m_bus == nullptr, return);
  m_bus = &bus;
}

//--------------------------------------------------------------------------------------------------
// Playback status queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks if an MDF4 file is currently open (decoded payload adopted).
 */
bool MDF4::Player::isOpen() const
{
  return m_open;
}

/**
 * @brief Checks if playback is currently active
 */
bool MDF4::Player::isPlaying() const
{
  return m_playing;
}

/**
 * @brief Returns whether the background decode is still running.
 */
bool MDF4::Player::indexing() const
{
  return m_decoding;
}

/**
 * @brief Returns background-decode progress in the range 0.0 to 1.0.
 */
double MDF4::Player::indexProgress() const
{
  return std::clamp(m_decodeProgress, 0.0, 1.0);
}

/**
 * @brief Returns the total number of frames in the file
 */
int MDF4::Player::frameCount() const
{
  return static_cast<int>(m_timestamps.size());
}

/**
 * @brief Returns the current playback progress
 */
double MDF4::Player::progress() const
{
  if (frameCount() == 0)
    return 0.0;

  return static_cast<double>(framePosition()) / frameCount();
}

/**
 * @brief Returns the filename of the currently open file
 */
QString MDF4::Player::filename() const
{
  if (isOpen()) {
    auto fileInfo = QFileInfo(m_filePath);
    return fileInfo.fileName();
  }

  return "";
}

/**
 * @brief Returns the current frame position
 */
int MDF4::Player::framePosition() const
{
  return m_framePos;
}

/**
 * @brief Returns the current playback timestamp
 */
const QString& MDF4::Player::timestamp() const
{
  return m_timestamp;
}

//--------------------------------------------------------------------------------------------------
// Playback control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts real-time playback from current position
 */
void MDF4::Player::play()
{
  if (!isOpen())
    return;

  if (m_framePos >= frameCount() - 1)
    m_framePos = 0;

  (void)m_engine.nextEpoch();
  m_startTimestamp = m_timestamps[m_framePos];
  m_elapsedTimer.start();
  m_engine.stopSeek();

  anchorSteadyBase(m_framePos);
  m_playing = true;
  Q_EMIT playerStateChanged();
}

/**
 * @brief Pauses playback at current position
 */
void MDF4::Player::pause()
{
  (void)m_engine.nextEpoch();
  m_playing = false;
  Q_EMIT playerStateChanged();
}

/**
 * @brief Toggles between play and pause states
 */
void MDF4::Player::toggle()
{
  if (isPlaying())
    pause();
  else
    play();
}

//--------------------------------------------------------------------------------------------------
// File operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens a file selection dialog for MDF4 files
 */
void MDF4::Player::openFile()
{
  auto& workspaceManager = Core::services().workspaceManager;
  auto* dialog           = new QFileDialog(qApp->activeWindow(),
                                 tr("Select MDF4 file"),
                                 workspaceManager.path("MDF4"),
                                 tr("MDF4 files (*.mf4 *.dat)"));

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
 * @brief Opens an MDF4 file from the specified path: after the license and connection gates,
 *        the whole decode runs on the background worker (spec 0022) and the player becomes
 *        open when the columnar payload lands.
 */
void MDF4::Player::openFile(const QString& filePath)
{
#ifdef BUILD_COMMERCIAL
  const auto& token   = Licensing::CommercialToken::current();
  const bool licensed = token.isValid() && SS_LICENSE_GUARD();
#else
  const bool licensed = false;
#endif

  if (!licensed) {
    Core::Prompt::showMessageBox(
      tr("MDF4 Playback is a Pro feature."),
      tr("Activate Serial Studio Pro or start the free trial to enable MDF4 playback."));
    return;
  }

  if (!Replay::ensureLinkReleased(
        m_bus,
        tr("Disconnect from device?"),
        tr("You must disconnect from the current device before opening a MDF4 file."),
        Core::Prompt::Question,
        QString()))
    return;

  closeFile();
  startDecoding(filePath);
}

/**
 * @brief Closes the currently open file (or cancels an in-flight decode) and releases
 *        resources.
 */
void MDF4::Player::closeFile()
{
  // code-verify off
  // A close reaching this while an inject is on the stack would free m_text under the borrowed
  // cell pointers already handed to the builder. Re-queue instead.
  // code-verify on
  if (m_injecting) {
    QMetaObject::invokeMethod(this, [this] { closeFile(); }, Qt::QueuedConnection);
    return;
  }

  const bool was_open     = m_open;
  const bool was_decoding = (m_loaderThread != nullptr);
  if (!was_open && !was_decoding)
    return;

  m_playing  = false;
  m_framePos = 0;
  m_engine.stopSeek();

  ++m_decodeGeneration;
  stopDecoding();

  m_open = false;
  m_filePath.clear();
  m_timestamp.clear();
  m_channelNames    = QStringList();
  // code-verify off
  // Default-assign, not clear(): std::vector::clear keeps capacity and QList keeps its
  // allocation, so a closed multi-gigabyte recording would pin its index storage until the next
  // open. The columnar payload is the player's entire footprint -- give it back (spec 0064).
  // code-verify on
  m_channelIsString = std::vector<bool>();
  m_timestamps      = std::vector<double>();
  m_numeric         = std::vector<std::vector<double>>();
  m_text            = std::vector<std::vector<QString>>();
  m_active          = std::vector<std::vector<bool>>();
  m_rowSourceBits   = QVector<quint8>();
  m_bitSourceIds.clear();
  m_lastSourceRow.clear();
  m_multiSource    = false;
  m_decodeProgress = 0.0;
  m_seekColumnByKey.clear();
  m_seekColumnByKey.squeeze();
  m_sourceChannelsByIndex.clear();
  m_typedCells = {};

  auto& frameBuilder = DataModel::pipelineModules().frameBuilder;
  frameBuilder.registerQuickPlotHeaders(QStringList());
  frameBuilder.setReplayColumnMap({});

  if (was_open) {
    if (m_bus)
      m_bus->publishState<Core::Bus::ReplayPlayerStateChanged>(1, isOpen());

    Q_EMIT openChanged();
    Q_EMIT playerStateChanged();
  }

  Q_EMIT indexingChanged();
}

/**
 * @brief Starts the background decode of @p filePath on a fresh worker thread.
 */
void MDF4::Player::startDecoding(const QString& filePath)
{
  SS_ASSERT(!filePath.isEmpty(), return);
  SS_ASSERT(m_loaderThread == nullptr, return);

  ++m_decodeGeneration;
  m_loaderThread = new QThread(this);
  m_loaderThread->setObjectName(QStringLiteral("MDF4::PlayerLoader"));
  m_loader = new PlayerLoaderWorker();
  m_loader->moveToThread(m_loaderThread);

  m_loaderLinks = {
    connect(m_loader,
            &PlayerLoaderWorker::progressUpdate,
            this,
            &MDF4::Player::onDecodeProgress,
            Qt::QueuedConnection),
    connect(m_loader,
            &PlayerLoaderWorker::finished,
            this,
            &MDF4::Player::onDecodeFinished,
            Qt::QueuedConnection),
  };

  m_loaderThread->start();

  m_decoding       = true;
  m_decodeProgress = 0.0;

  auto* loader             = m_loader;
  const quint64 generation = m_decodeGeneration;
  QMetaObject::invokeMethod(
    loader,
    [loader, filePath, generation, bits = buildChannelSourceBits()]() {
      loader->decodeFile(filePath, generation, bits);
    },
    Qt::QueuedConnection);

  Q_EMIT indexingChanged();
}

/**
 * @brief Cancels and joins the decode thread; leaks it on a join timeout rather than
 *        destroying a running thread.
 */
void MDF4::Player::stopDecoding()
{
  if (!m_loaderThread)
    return;

  constexpr int kJoinTimeoutMs = 5000;

  if (m_loader)
    m_loader->requestCancel();

  m_loaderThread->quit();
  if (m_loaderThread->wait(kJoinTimeoutMs)) {
    delete m_loader;
    delete m_loaderThread;
  } else {
    qWarning() << "[MDF4::Player] Decode thread did not stop in time; detaching it.";
    for (const auto& link : std::as_const(m_loaderLinks))
      disconnect(link);

    m_loaderThread->setParent(nullptr);
    connect(m_loaderThread, &QThread::finished, m_loader, &QObject::deleteLater);
    connect(m_loaderThread, &QThread::finished, m_loaderThread, &QObject::deleteLater);
  }

  m_loaderLinks.clear();
  m_loader       = nullptr;
  m_loaderThread = nullptr;
  m_decoding     = false;
}

/**
 * @brief Refreshes the decode-progress property from a worker update.
 */
void MDF4::Player::onDecodeProgress(double fraction, quint64 generation)
{
  if (generation != m_decodeGeneration)
    return;

  m_decodeProgress = fraction;
  Q_EMIT indexingChanged();
}

/**
 * @brief Adopts a finished decode payload: on success the player opens with the columnar
 *        data; errors surface the worker's message; stale/cancelled payloads are dropped.
 */
void MDF4::Player::onDecodeFinished(const MDF4::PlayerDecodePayloadPtr& payload)
{
  SS_ASSERT(payload != nullptr, return);

  if (payload->generation != m_decodeGeneration)
    return;

  stopDecoding();
  m_decodeProgress = 1.0;

  if (payload->cancelled) {
    Q_EMIT indexingChanged();
    return;
  }

  if (!payload->ok) {
    Core::Prompt::showMessageBox(payload->errorTitle, payload->errorBody, Core::Prompt::Critical);
    Q_EMIT indexingChanged();
    return;
  }

  if (payload->timestamps.empty() || payload->channelNames.isEmpty()) {
    Core::Prompt::showMessageBox(tr("No data in file"),
                                 tr("The MDF4 file contains no measurement data."),
                                 Core::Prompt::Critical);
    Q_EMIT indexingChanged();
    return;
  }

  m_filePath        = payload->filePath;
  m_channelNames    = std::move(payload->channelNames);
  m_channelIsString = std::move(payload->channelIsString);
  m_timestamps      = std::move(payload->timestamps);
  m_numeric         = std::move(payload->numeric);
  m_text            = std::move(payload->text);
  m_active          = std::move(payload->active);
  m_rowSourceBits   = std::move(payload->rowSourceBits);
  m_lastSourceRow   = QVector<int>(m_bitSourceIds.size(), -1);

  m_open     = true;
  m_framePos = 0;

  sendHeaderFrame();

  if (m_bus)
    m_bus->publishState<Core::Bus::ReplayPlayerStateChanged>(1, isOpen());

  Q_EMIT openChanged();
  Q_EMIT playerStateChanged();
  Q_EMIT indexingChanged();

  if (payload->partialData) [[unlikely]]
    Core::Prompt::showMessageBox(
      tr("MDF4 data may be incomplete"),
      tr("Part of the file's data section could not be read; the recording may be truncated."),
      Core::Prompt::Warning);
}

/**
 * @brief Advances to the next frame
 */
void MDF4::Player::nextFrame()
{
  if (!isOpen())
    return;

  if (isPlaying())
    pause();

  if (m_framePos < frameCount() - 1) {
    ++m_framePos;

    auto& dashboard = plotSink();
    dashboard.clearPlotData();

    int framesToLoad = dashboard.points();
    int startFrame   = std::max(0, m_framePos - framesToLoad);
    int endFrame     = std::min(frameCount() - 1, m_framePos);

    processFrameBatch(startFrame, endFrame);
    updateData();
  }
}

/**
 * @brief Steps back to the previous frame
 */
void MDF4::Player::previousFrame()
{
  if (!isOpen())
    return;

  if (isPlaying())
    pause();

  if (m_framePos > 0) {
    --m_framePos;

    auto& dashboard = plotSink();
    dashboard.clearPlotData();

    int framesToLoad = dashboard.points();
    int startFrame   = std::max(0, m_framePos - framesToLoad);
    int endFrame     = std::min(frameCount() - 1, m_framePos);

    processFrameBatch(startFrame, endFrame);
    updateData();
  }
}

//--------------------------------------------------------------------------------------------------
// Progress & seeking
//--------------------------------------------------------------------------------------------------

/**
 * @brief Seeks to a normalized position (tape scrub, spec 0020): the position and timestamp
 *        update immediately, a coalescing timer live-fills the plots at ~30 Hz, and the settle
 *        timer runs the exact full-window rebuild once the slider rests.
 */
void MDF4::Player::setProgress(const double progress)
{
  if (!isOpen())
    return;

  if (isPlaying())
    pause();

  const int newFramePos = qBound(0, static_cast<int>(progress * frameCount()), frameCount() - 1);
  if (newFramePos == m_framePos)
    return;

  m_framePos = newFramePos;
  if (m_framePos < frameCount()) {
    m_timestamp = DataModel::ReplayPlaybackEngine::formatTimestamp(m_timestamps[m_framePos]);
    Q_EMIT timestampChanged();
  }

  m_engine.armSeek();
}

/**
 * @brief First row of the scrub window ending at @p target: walks back until the plot time
 *        range is covered (never fewer than points() rows), capped by the engine so
 *        dense recordings bound the per-tick cost.
 */
int MDF4::Player::seekWindowStartRow(int target) const
{
  SS_ASSERT(target >= 0, return 0);
  SS_ASSERT(target < frameCount(), return qMax(0, frameCount() - 1));

  auto& dashboard = plotSink();
  return DataModel::ReplayPlaybackEngine::seekWindowStartRow(
    target, dashboard.points(), dashboard.plotTimeRange(), [this](int row) {
      return m_timestamps[static_cast<size_t>(row)];
    });
}

/**
 * @brief One coalesced scrub tick: bulk-fills the plot rings from the trailing window ending
 *        at the cursor and injects the cursor row so scalar widgets track it. Without a seek
 *        column map the settle rebuild runs instead -- a bulk fill with an empty series map
 *        would wipe the rings and blank the plots for the whole drag.
 */
void MDF4::Player::performSeekTick()
{
  if (!isOpen() || isPlaying())
    return;

  SS_ASSERT(m_framePos >= 0, return);
  SS_ASSERT(m_framePos < frameCount(), return);

  if (m_seekColumnByKey.isEmpty()) {
    performSeekSettle();
    return;
  }

  auto& dashboard  = plotSink();
  const int target = m_framePos;
  const int start  = seekWindowStartRow(target);

  QVector<double> times;
  QHash<qint64, QVector<double>> series;
  buildSeekWindow(start, target, times, series);
  dashboard.bulkLoadPlotWindow(times, series);

  anchorSteadyBase(target);
  injectRow(target);
}

/**
 * @brief At-rest settle pass: exact trailing-window replay through the fast lane (FFT and the
 *        other frame-fed widgets), then a full-time-window bulk fill so the plots keep the
 *        complete tape view instead of collapsing to the pipeline batch.
 */
void MDF4::Player::performSeekSettle()
{
  if (!isOpen() || isPlaying())
    return;

  SS_ASSERT(m_framePos >= 0, return);
  SS_ASSERT(m_framePos < frameCount(), return);

  auto& dashboard = plotSink();
  dashboard.clearPlotData();

  const int window = qMin(dashboard.points(), m_framePos + 1);
  const int start  = qMax(0, m_framePos - window + 1);
  processFrameBatch(start, m_framePos);
  m_lastSourceRow.fill(-1);
  backfillSparseSources();

  if (!m_seekColumnByKey.isEmpty()) {
    QVector<double> times;
    QHash<qint64, QVector<double>> series;
    const int fillStart = seekWindowStartRow(m_framePos);
    buildSeekWindow(fillStart, m_framePos, times, series);
    dashboard.bulkLoadPlotWindow(times, series);
  }

  updateData();
}

/**
 * @brief Forward-fills NaN gaps in a seek series and backfills the leading run from the
 *        first stored value (sparse channel groups leave frames inactive; mirrors the
 *        Sessions player's fillMdf4SeekGaps so absent samples hold instead of dropping to 0).
 */
static void fillMdf4SeekGaps(QVector<double>& values)
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
 * @brief Fills the seek-window times and per-(source, uid) numeric series straight from the
 *        columnar channel vectors (already doubles, no text parse); inactive frames become
 *        NaN gaps that forward-fill, matching sample-and-hold replay semantics.
 */
void MDF4::Player::buildSeekWindow(int startRow,
                                   int endRow,
                                   QVector<double>& times,
                                   QHash<qint64, QVector<double>>& series)
{
  SS_ASSERT(startRow >= 0, return);
  SS_ASSERT(startRow <= endRow, return);
  SS_ASSERT(endRow < frameCount(), return);

  const int n = endRow - startRow + 1;
  times.resize(n);
  for (int k = 0; k < n; ++k) {
    times[k] = m_timestamps[static_cast<size_t>(startRow + k)];
    if (k > 0)
      times[k] = qMax(times[k], times[k - 1]);
  }

  auto& dashboard  = plotSink();
  const auto pairs = dashboard.replaySeekSeries();
  for (const auto& pair : pairs) {
    const qint64 key = DataModel::replaySeekKey(pair.first, pair.second);
    const int column = m_seekColumnByKey.value(key, -1);
    if (column < 0)
      continue;

    const bool numeric_col = column < static_cast<int>(m_numeric.size())
                          && column < static_cast<int>(m_channelIsString.size())
                          && !m_channelIsString[static_cast<size_t>(column)];

    auto& values = series[key];
    values.resize(n);
    for (int k = 0; k < n; ++k) {
      const auto row = static_cast<size_t>(startRow + k);
      const bool ok  = numeric_col && row < m_numeric[static_cast<size_t>(column)].size()
                   && channelActive(column, startRow + k);
      values[k] =
        ok ? m_numeric[static_cast<size_t>(column)][row] : std::numeric_limits<double>::quiet_NaN();
    }

    fillMdf4SeekGaps(values);
  }
}

//--------------------------------------------------------------------------------------------------
// Data processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Plays back toward the target timestamp, striding over intermediate frames when the
 *        backlog exceeds the per-pass inject budget so playback tracks real time instead of
 *        stretching (spec 0022 catch-up decimation); then schedules the NEXT frame with its
 *        real delay so the last caught-up frame is never injected twice.
 */
void MDF4::Player::catchUpToTarget(double targetTime)
{
  constexpr int kCatchUpMaxInjects = 512;
  constexpr int kCatchUpScanMax    = DataModel::ReplayPlaybackEngine::kCatchUpScanMax;
  const QDeadlineTimer budget(DataModel::ReplayPlaybackEngine::kCatchUpBudgetMs);

  int targetRow  = m_framePos;
  const int last = frameCount() - 1;
  for (int i = 0; i < kCatchUpScanMax && targetRow < last; ++i) {
    if (m_timestamps[static_cast<size_t>(targetRow + 1)] > targetTime)
      break;

    ++targetRow;
  }

  const int stride = qMax(1, (targetRow - m_framePos) / kCatchUpMaxInjects);
  for (int processed = 0;
       processed < kCatchUpMaxInjects && m_framePos < targetRow && !budget.hasExpired();
       ++processed) {
    if (!isOpen() || !isPlaying())
      break;

    m_framePos = qMin(targetRow, m_framePos + stride);
    sendFrame(m_framePos);
  }

  backfillSparseSources();

  if (stride > 2 && !m_seekColumnByKey.isEmpty() && m_engine.catchUpFillDue()) {
    auto& dashboard = plotSink();
    QVector<double> times;
    QHash<qint64, QVector<double>> series;
    buildSeekWindow(seekWindowStartRow(m_framePos), m_framePos, times, series);
    dashboard.bulkLoadPlotWindow(times, series);
  }

  if (isOpen() && m_framePos < frameCount()) {
    m_timestamp = DataModel::ReplayPlaybackEngine::formatTimestamp(
      m_timestamps[static_cast<size_t>(m_framePos)]);
    Q_EMIT timestampChanged();
  }

  if (!isPlaying())
    return;

  if (m_framePos >= frameCount() - 1) {
    pause();
    return;
  }

  constexpr double kInvMs      = 1.0 / 1000.0;
  constexpr double kMaxDelayMs = 86'400'000.0;
  const qint64 elapsedMs       = m_elapsedTimer.elapsed();
  const double nowTarget       = m_startTimestamp + (elapsedMs * kInvMs);
  const double nextTime        = m_timestamps[static_cast<size_t>(m_framePos + 1)];
  const double deltaMs         = (nextTime - nowTarget) * 1000.0;
  const qint64 delayMs =
    std::isfinite(deltaMs) ? static_cast<qint64>(std::clamp(deltaMs, 0.0, kMaxDelayMs)) : 0;
  const quint64 epoch = m_engine.epoch();
  QTimer::singleShot(delayMs, Qt::PreciseTimer, this, [this, epoch]() {
    if (isOpen() && isPlaying() && m_engine.isCurrentEpoch(epoch)) {
      ++m_framePos;
      updateData();
    }
  });
}

/**
 * @brief Updates current frame data and manages playback timing.
 */
void MDF4::Player::updateData()
{
  if (!isOpen())
    return;

  if (m_framePos >= 0 && m_framePos < frameCount()) {
    m_timestamp = DataModel::ReplayPlaybackEngine::formatTimestamp(
      m_timestamps[static_cast<size_t>(m_framePos)]);
    Q_EMIT timestampChanged();
  }

  if (!isPlaying())
    return;

  if (framePosition() >= frameCount() - 1) {
    pause();
    return;
  }

  sendFrame(m_framePos);

  constexpr double kInvMs      = 1.0 / 1000.0;
  constexpr double kMaxDelayMs = 86'400'000.0;
  const qint64 elapsedMs       = m_elapsedTimer.elapsed();
  const double targetTime      = m_startTimestamp + (elapsedMs * kInvMs);
  const double nextTime        = m_timestamps[static_cast<size_t>(framePosition() + 1)];

  if (nextTime <= targetTime) {
    catchUpToTarget(targetTime);
    return;
  }

  const double deltaMs = (nextTime - targetTime) * 1000.0;
  const qint64 delayMs =
    std::isfinite(deltaMs) ? static_cast<qint64>(std::clamp(deltaMs, 0.0, kMaxDelayMs)) : 0;
  const quint64 epoch = m_engine.epoch();
  QTimer::singleShot(delayMs, Qt::PreciseTimer, this, [this, epoch]() {
    if (isOpen() && isPlaying() && m_engine.isCurrentEpoch(epoch)) {
      ++m_framePos;
      updateData();
    }
  });
}

//--------------------------------------------------------------------------------------------------
// Frame transmission helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Processes a batch of frames for plot history
 */
void MDF4::Player::processFrameBatch(int startFrame, int endFrame)
{
  anchorSteadyBase(std::max(0, startFrame));
  for (int i = startFrame; i <= endFrame; ++i)
    if (i >= 0 && i < frameCount())
      sendFrame(i);
}

/**
 * @brief Sends a single frame to the IO manager for processing
 */
void MDF4::Player::sendFrame(int frameIndex)
{
  if (!isOpen() || frameIndex < 0 || frameIndex >= frameCount())
    return;

  injectRow(frameIndex);
}

/**
 * @brief Registers MDF4 channel names with Quick Plot
 */
void MDF4::Player::sendHeaderFrame()
{
  if (m_channelNames.isEmpty())
    return;

  auto& appState = DataModel::pipelineModules().appState;
  if (appState.operationMode() == SerialStudio::ProjectFile) {
    buildReplayLayout();
    if (m_multiSource)
      return;
  }

  QStringList headers;
  headers.reserve(m_channelNames.size());
  for (int i = 0; i < m_channelNames.size(); ++i) {
    QString name = m_channelNames.at(i);
    if (name.isEmpty())
      name = QString("Channel_%1").arg(i + 1);

    headers.append(name);
  }

  if (appState.operationMode() != SerialStudio::ProjectFile) {
    m_seekColumnByKey.clear();
    for (int i = 0; i < headers.size(); ++i) {
      m_seekColumnByKey.insert(DataModel::replaySeekKey(0, DataModel::dataset_unique_id(0, 0, i)),
                               i);
      m_seekColumnByKey.insert(DataModel::replaySeekKey(0, DataModel::dataset_unique_id(0, 1, i)),
                               i);
    }
  }

  auto& frameBuilder = DataModel::pipelineModules().frameBuilder;
  frameBuilder.registerQuickPlotHeaders(headers);
}

//--------------------------------------------------------------------------------------------------
// Date/time operations
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Frame building
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the recorded activity bit for (channel, frame); missing entries count as
 *        inactive.
 */
bool MDF4::Player::channelActive(int channel, int frameIndex) const
{
  if (channel < 0 || channel >= static_cast<int>(m_active.size()))
    return false;

  const auto& column = m_active[static_cast<size_t>(channel)];
  if (frameIndex < 0 || frameIndex >= static_cast<int>(column.size()))
    return false;

  return column[static_cast<size_t>(frameIndex)];
}

/**
 * @brief Fills @p cells with the row at @p index for the byte path. String-typed channels
 *        replay their cached text verbatim; numeric channels format the cached double with
 *        the same 'g'/10 rendering the typed lane uses.
 */
bool MDF4::Player::buildRowCells(int index, QStringList& cells) const
{
  if (!isOpen() || index < 0 || index >= frameCount())
    return false;

  const auto row            = static_cast<size_t>(index);
  const size_t channelCount = m_channelIsString.size();

  cells.clear();
  cells.reserve(static_cast<int>(channelCount));

  for (size_t c = 0; c < channelCount; ++c) {
    const bool is_string = m_channelIsString[c];
    if (is_string && c < m_text.size() && row < m_text[c].size())
      cells.append(m_text[c][row]);
    else if (!is_string && c < m_numeric.size() && row < m_numeric[c].size())
      cells.append(QString::number(m_numeric[c][row], 'g', 10));
    else
      cells.append(QStringLiteral("0"));
  }

  return true;
}

/**
 * @brief Fills @p cells with typed replay cells for the row at @p index: numeric channels
 *        pass the native double (spec 0022 R7), string channels borrow the cached text.
 */
qsizetype MDF4::Player::buildRowCellsTyped(
  int index, QVarLengthArray<DataModel::FrameBuilder::ReplayCell, 128>& cells) const
{
  SS_ASSERT(index >= 0, return 0);
  SS_ASSERT(index < frameCount(), return 0);

  const auto row            = static_cast<size_t>(index);
  const size_t channelCount = m_channelIsString.size();

  cells.clear();
  cells.reserve(static_cast<qsizetype>(channelCount));

  for (size_t c = 0; c < channelCount; ++c) {
    DataModel::FrameBuilder::ReplayCell cell{nullptr, 0.0};
    const bool is_string = m_channelIsString[c];
    if (is_string && c < m_text.size() && row < m_text[c].size())
      cell.text = &m_text[c][row];
    else if (!is_string && c < m_numeric.size() && row < m_numeric[c].size())
      cell.number = m_numeric[c][row];

    cells.append(cell);
  }

  return cells.size();
}

/**
 * @brief Extracts a frame of data at the specified index as a joined replay row.
 */
QByteArray MDF4::Player::getFrame(const int index)
{
  QStringList cells;
  if (!buildRowCells(index, cells))
    return QByteArray();

  QByteArray frame = DataModel::joinReplayRow(cells);
  frame.append('\n');
  return frame;
}

/**
 * @brief Anchors the steady-clock base used to stamp replayed rows with recorded deltas.
 */
void MDF4::Player::anchorSteadyBase(int frameIndex)
{
  SS_ASSERT(frameIndex >= 0, frameIndex = 0);

  m_engine.anchorSteadyBase(
    (frameIndex < frameCount()) ? m_timestamps[static_cast<size_t>(frameIndex)] : 0.0);
}

/**
 * @brief Steady timestamp for @p frameIndex: the anchored base advanced by the recorded delta,
 *        so the recording -- not the wall clock -- owns replay time.
 */
std::chrono::steady_clock::time_point MDF4::Player::rowSteadyTimestamp(int frameIndex) const
{
  if (frameIndex < 0 || frameIndex >= frameCount())
    return m_engine.steadyBase();

  return m_engine.steadyTimestampFor(m_timestamps[static_cast<size_t>(frameIndex)]);
}

/**
 * @brief Per-channel source bit table for the loader (channel order == project traversal, the
 *        writer's order). Also seeds m_bitSourceIds; empty outside ProjectFile mode.
 */
QVector<quint8> MDF4::Player::buildChannelSourceBits()
{
  m_bitSourceIds.clear();

  auto& appState = DataModel::pipelineModules().appState;
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return {};

  QVector<quint8> bits;
  auto& projectModel = DataModel::pipelineModules().projectModel;
  for (const auto& g : projectModel.groups()) {
    if (g.widget == QLatin1String("image"))
      continue;

    int bit = static_cast<int>(m_bitSourceIds.indexOf(g.sourceId));
    if (bit < 0 && m_bitSourceIds.size() < 8) {
      bit = m_bitSourceIds.size();
      m_bitSourceIds.append(g.sourceId);
    }

    for (int i = 0; i < static_cast<int>(g.datasets.size()); ++i)
      bits.append(bit >= 0 ? static_cast<quint8>(1u << bit) : quint8(0));
  }

  return bits;
}

/**
 * @brief Injects @p frameIndex for @p sourceId only: the backfill path updates one stale source
 *        from its own latest active row without re-publishing every other source.
 */
void MDF4::Player::injectSourceRow(int frameIndex, int sourceId)
{
  SS_ASSERT(frameIndex >= 0, return);
  SS_ASSERT(frameIndex < frameCount(), return);

  if (m_injecting)
    return;

  const QScopedValueRollback<bool> reentry_guard(m_injecting, true);
  const qsizetype count = buildRowCellsTyped(frameIndex, m_typedCells);
  if (count <= 0)
    return;

  const auto it = m_sourceChannelsByIndex.constFind(sourceId);
  if (it == m_sourceChannelsByIndex.constEnd())
    return;

  auto& frameBuilder     = DataModel::pipelineModules().frameBuilder;
  const auto timestamp   = rowSteadyTimestamp(frameIndex);
  const auto& orderedChs = it.value();
  QVarLengthArray<DataModel::FrameBuilder::ReplayCell, 128> cells;
  cells.reserve(orderedChs.size());
  for (int ch : orderedChs) {
    static const QString kEmpty;
    DataModel::FrameBuilder::ReplayCell cell{&kEmpty, 0.0};
    if (ch >= 0 && ch < count)
      cell = m_typedCells.at(ch);

    cells.append(cell);
  }

  frameBuilder.replayChannelsTyped(sourceId, cells.constData(), cells.size(), timestamp);
}

/**
 * @brief Brings every source the strided catch-up skipped up to its latest active row at or
 *        before the playhead (spec 0064): a mixed-rate recording holds a slow source's samples
 *        on a handful of rows per second, so row-strided sampling almost never lands on one and
 *        its widgets would otherwise freeze for the whole replay.
 */
void MDF4::Player::backfillSparseSources()
{
  constexpr int kBackfillScanMax = 262144;

  if (!m_multiSource || m_bitSourceIds.isEmpty())
    return;

  const int last = qMin(m_framePos, static_cast<int>(m_rowSourceBits.size()) - 1);
  if (last < 0)
    return;

  for (int b = 0; b < m_bitSourceIds.size(); ++b) {
    const auto mask = static_cast<quint8>(1u << b);
    const int stop  = qMax(0, last - kBackfillScanMax);
    int found       = -1;
    for (int r = last; r >= stop; --r) {
      if (m_rowSourceBits.at(r) & mask) {
        found = r;
        break;
      }
    }

    if (found < 0 || found == m_lastSourceRow.value(b, -1))
      continue;

    m_lastSourceRow[b] = found;
    injectSourceRow(found, m_bitSourceIds.at(b));
  }
}

/**
 * @brief Replays one indexed row through the FrameBuilder typed replay lane (spec 0022):
 *        numeric channels flow as native doubles with the recorded timestamp -- no per-cell
 *        number formatting. QuickPlot mode keeps the byte path.
 */
void MDF4::Player::injectRow(int frameIndex)
{
  SS_ASSERT(frameIndex >= 0, return);
  SS_ASSERT(frameIndex < frameCount(), return);

  // code-verify off
  // The replay marshal is a plain BlockingQueuedConnection: it does NOT run this thread's event
  // loop (dataflow.md). The latch is what makes closeFile() re-queue instead of freeing m_text
  // and m_sourceChannelsByIndex under an inject already on the stack.
  // code-verify on
  if (m_injecting)
    return;

  const QScopedValueRollback<bool> reentry_guard(m_injecting, true);

  auto& appState = DataModel::pipelineModules().appState;
  if (appState.operationMode() != SerialStudio::ProjectFile) {
    injectFrame(getFrame(frameIndex), frameIndex);
    return;
  }

  const qsizetype count = buildRowCellsTyped(frameIndex, m_typedCells);
  if (count <= 0) [[unlikely]]
    return;

  auto& frameBuilder   = DataModel::pipelineModules().frameBuilder;
  const auto timestamp = rowSteadyTimestamp(frameIndex);

  if (!m_multiSource) {
    frameBuilder.replayChannelsTyped(0, m_typedCells.constData(), count, timestamp);
    return;
  }

  // code-verify off
  // Iterate a copy: a close re-queued by the latch above still runs before this loop's next
  // iteration on a later turn. Implicitly shared, so the copy costs one refcount.
  // code-verify on
  const auto sourceChannels = m_sourceChannelsByIndex;
  for (auto it = sourceChannels.constBegin(); it != sourceChannels.constEnd(); ++it) {
    const auto& orderedChs = it.value();
    QVarLengthArray<DataModel::FrameBuilder::ReplayCell, 128> sourceCells;
    sourceCells.reserve(orderedChs.size());
    bool anyActive = false;
    for (int ch : orderedChs) {
      static const QString kEmpty;
      DataModel::FrameBuilder::ReplayCell cell{&kEmpty, 0.0};
      if (ch >= 0 && ch < count)
        cell = m_typedCells.at(ch);

      sourceCells.append(cell);
      anyActive = anyActive || channelActive(ch, frameIndex);
    }

    if (anyActive)
      frameBuilder.replayChannelsTyped(
        it.key(), sourceCells.constData(), sourceCells.size(), timestamp);
  }
}

//--------------------------------------------------------------------------------------------------
// Multi-source playback helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a channel-to-source mapping for multi-source MDF4 playback.
 */
void MDF4::Player::buildReplayLayout()
{
  m_seekColumnByKey.clear();
  m_sourceChannelsByIndex.clear();

  struct ChMeta {
    int uid;
    int sourceId;
  };

  QVector<ChMeta> chs;
  auto& projectModel = DataModel::pipelineModules().projectModel;
  for (const auto& g : projectModel.groups()) {
    if (g.widget == QLatin1String("image"))
      continue;

    for (const auto& d : g.datasets)
      chs.append({d.uniqueId, g.sourceId});
  }

  std::unordered_map<int, int> localCounter;
  for (const auto& m : chs)
    localCounter[m.sourceId];

  for (int ch = 0; ch < chs.size(); ++ch)
    m_seekColumnByKey.insert(DataModel::replaySeekKey(chs[ch].sourceId, chs[ch].uid), ch);

  m_multiSource = localCounter.size() > 1;

  std::unordered_map<int, std::unordered_map<int, int>> replay;

  if (!m_multiSource) {
    for (int ch = 0; ch < chs.size(); ++ch)
      replay[0][chs[ch].uid] = ch;
  }

  else {
    for (auto& kv : localCounter)
      kv.second = 0;

    for (int ch = 0; ch < chs.size(); ++ch) {
      m_sourceChannelsByIndex[chs[ch].sourceId].append(ch);
      replay[chs[ch].sourceId][chs[ch].uid] = localCounter[chs[ch].sourceId]++;
    }
  }

  auto& frameBuilder = DataModel::pipelineModules().frameBuilder;
  frameBuilder.setReplayColumnMap(std::move(replay));
}

/**
 * @brief Injects an MDF4 frame through the appropriate pipeline path.
 */
void MDF4::Player::injectFrame(const QByteArray& frame, int frameIndex)
{
  if (frame.isEmpty())
    return;

  if (!m_multiSource) {
    auto& connectionManager = payloadInjector();
    connectionManager.processPayload(frame);
    return;
  }

  const QString row = QString::fromUtf8(frame).trimmed();
  const auto fields = DataModel::splitReplayRow(row);

  QMap<int, QStringList> sourceFields;
  QMap<int, bool> sourceHasActive;
  for (auto it = m_sourceChannelsByIndex.constBegin(); it != m_sourceChannelsByIndex.constEnd();
       ++it) {
    const int srcId        = it.key();
    const auto& orderedChs = it.value();
    QStringList orderedCells;
    orderedCells.reserve(orderedChs.size());
    bool anyActive = false;
    for (int ch : orderedChs) {
      const QString cell = (ch >= 0 && ch < fields.size()) ? fields[ch] : QString();
      orderedCells.append(cell);
      anyActive = anyActive || channelActive(ch, frameIndex);
    }
    sourceFields.insert(srcId, std::move(orderedCells));
    sourceHasActive.insert(srcId, anyActive || frameIndex < 0);
  }

  QMap<int, QByteArray> sourcePayloads;
  for (auto it = sourceFields.constBegin(); it != sourceFields.constEnd(); ++it) {
    if (!sourceHasActive.value(it.key(), true))
      continue;

    sourcePayloads[it.key()] = DataModel::joinReplayRow(it.value()) + '\n';
  }

  if (!sourcePayloads.isEmpty()) {
    auto& connectionManager = payloadInjector();
    connectionManager.processMultiSourcePayload(frame, sourcePayloads);
  }
}

//--------------------------------------------------------------------------------------------------
// Event handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles keyboard shortcuts for playback control
 */
bool MDF4::Player::handleKeyPress(QKeyEvent* keyEvent)
{
  if (!isOpen())
    return false;

  if (keyEvent->key() == Qt::Key_Space) {
    toggle();
    return true;
  }

  else if (keyEvent->key() == Qt::Key_Left) {
    previousFrame();
    return true;
  }

  else if (keyEvent->key() == Qt::Key_Right) {
    nextFrame();
    return true;
  }

  return false;
}

/**
 * @brief Event filter for capturing keyboard events
 */

/**
 * @brief Captures key events and routes playback shortcuts to handleKeyPress.
 */
bool MDF4::Player::eventFilter(QObject* obj, QEvent* event)
{
  if (isOpen() && event->type() == QEvent::KeyPress) {
    auto* keyEvent = static_cast<QKeyEvent*>(event);
    if (!DataModel::ReplayPlaybackEngine::playbackKeyIsClaimed(keyEvent->key()))
      return handleKeyPress(keyEvent);
  }

  return QObject::eventFilter(obj, event);
}

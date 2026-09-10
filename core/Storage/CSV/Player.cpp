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

#include <cmath>
#include <cstring>
#include <limits>
#include <QApplication>
#include <QDateTime>
#include <QDeadlineTimer>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QGuiApplication>
#include <QInputDialog>
#include <QMap>
#include <QScopedValueRollback>
#include <QSet>
#include <QTimer>
#include <QtMath>
#include <QVarLengthArray>
#include <vector>

#include "AppState.h"
#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/DataModel/ExportSchema.h"
#include "Core/IO/IPayloadInjector.h"
#include "Core/Prompt/UserPrompt.h"
#include "Core/SerialStudio.h"
#include "Core/Services.h"
#include "Core/SSAssert.h"
#include "Core/WorkspaceManager.h"
#include "CSV/Player/RowSyntax.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/IReplayPlotSink.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"
#include "Replay/LinkGate.h"

static constexpr double kCsvInvMs       = 1.0 / 1000.0;
static constexpr int kDefaultIntervalMs = 1000;

//--------------------------------------------------------------------------------------------------
// Local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when no user can answer a modal dialog (headless/offscreen, e.g. --headless with the
 *        API server, the spec-0044 verifier child, CI). Callers then take the legacy default:
 *        a blocking prompt here would wedge the API dispatch that asked for the open.
 */
static bool nonInteractive()
{
  return !qApp || qApp->platformName() == QLatin1String("offscreen");
}

/**
 * @brief Forward-fills NaN gaps in a seek series and backfills the leading run from the
 *        first stored value (sparse rows leave most columns empty; mirrors the Sessions
 *        player's fillSeekGaps so absent cells hold the last value instead of dropping to 0).
 */
static void fillSeekGaps(QVector<double>& values)
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
 * @brief One seek-window cell as a double: NaN for a missing, empty or non-numeric cell so
 *        sparse rows become forward-fillable gaps instead of zeros.
 */
[[nodiscard]] static double seekCellValue(const QByteArrayView* spans, qsizetype count, int column)
{
  SS_ASSERT(spans != nullptr || count == 0, return std::numeric_limits<double>::quiet_NaN());
  SS_ASSERT(column >= 0, return std::numeric_limits<double>::quiet_NaN());

  if (column >= count || spans[column].isEmpty())
    return std::numeric_limits<double>::quiet_NaN();

  bool ok             = false;
  const double parsed = SerialStudio::toDouble(spans[column], &ok);
  return ok ? parsed : std::numeric_limits<double>::quiet_NaN();
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the CSV player and installs the global key-event filter.
 */
CSV::Player::Player()
  : m_framePos(0)
  , m_injecting(false)
  , m_playing(false)
  , m_pausedAtFrontier(false)
  , m_timestamp("")
  , m_mapped(nullptr)
  , m_mappedSize(0)
  , m_dataOffset(0)
  , m_timeScale(1.0)
  , m_intervalSeconds(0.0)
  , m_anchorMs(0)
  , m_startSeconds(-1.0)
  , m_bus(nullptr)
  , m_plotSink(nullptr)
  , m_payloadInjector(nullptr)
{
  qApp->installEventFilter(this);
  qRegisterMetaType<CSV::PlayerIndexRequestPtr>();
  qRegisterMetaType<CSV::PlayerIndexBatchPtr>();

  connect(
    &m_engine, &DataModel::ReplayPlaybackEngine::seekTick, this, &CSV::Player::performSeekTick);
  connect(
    &m_engine, &DataModel::ReplayPlaybackEngine::seekSettle, this, &CSV::Player::performSeekSettle);
  connect(&m_indexer, &CSV::FileIndexer::batchReady, this, &CSV::Player::onIndexBatch);
  connect(&m_indexer, &CSV::FileIndexer::finished, this, &CSV::Player::onIndexFinished);
}

/**
 * @brief Destructor - joins any in-flight indexer so the worker never outlives the mapping.
 */
CSV::Player::~Player()
{
  (void)stopIndexing();
}

/**
 * @brief Returns the singleton CSV Player instance.
 */
CSV::Player& CSV::Player::instance()
{
  static Player singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Playback status queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether a CSV file is currently open.
 */
bool CSV::Player::isOpen() const
{
  return m_csvFile && m_csvFile->isOpen();
}

/**
 * @brief Returns the CSV playback progress in the range 0.0 to 1.0.
 */
double CSV::Player::progress() const
{
  const auto count = frameCount();
  if (count <= 0)
    return 0.0;

  return static_cast<double>(framePosition()) / count;
}

/**
 * @brief Returns whether playback is currently active.
 */
bool CSV::Player::isPlaying() const
{
  return m_playing;
}

/**
 * @brief Returns whether the background indexer is still scanning the file.
 */
bool CSV::Player::indexing() const
{
  return m_indexer.indexing();
}

/**
 * @brief Returns background-indexing progress in the range 0.0 to 1.0.
 */
double CSV::Player::indexProgress() const
{
  return m_indexer.progress();
}

//--------------------------------------------------------------------------------------------------
// Frame information
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the number of indexed data rows (grows while indexing runs).
 */
int CSV::Player::frameCount() const
{
  return static_cast<int>(m_rowOffsets.size());
}

/**
 * @brief Returns the current CSV row being replayed.
 */
int CSV::Player::framePosition() const
{
  return m_framePos;
}

/**
 * @brief Returns the base filename of the currently open CSV file.
 */
QString CSV::Player::filename() const
{
  if (isOpen()) {
    auto fileInfo = QFileInfo(m_csvFile->fileName());
    return fileInfo.fileName();
  }

  return "";
}

/**
 * @brief Returns the formatted timestamp of the current frame.
 */
const QString& CSV::Player::timestamp() const
{
  return m_timestamp;
}

//--------------------------------------------------------------------------------------------------
// Playback control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts CSV playback at the original capture speed; a play request that reaches an
 *        empty (still-indexing) frontier arms the auto-resume instead.
 */
void CSV::Player::play()
{
  SS_ASSERT(isOpen(), return);

  if (frameCount() <= 0) {
    m_pausedAtFrontier = indexing();
    return;
  }

  if (m_framePos >= frameCount() - 1 && !indexing())
    m_framePos = 0;

  m_pausedAtFrontier = false;
  (void)m_engine.nextEpoch();
  m_startSeconds = rowSecondsSinceStart(m_framePos);
  m_elapsedTimer.start();
  m_engine.stopSeek();

  anchorSteadyBase(m_framePos);
  m_playing = true;
  Q_EMIT playerStateChanged();
  updateData();
}

/**
 * @brief Pauses CSV playback.
 */
void CSV::Player::pause()
{
  SS_ASSERT(isOpen(), return);

  (void)m_engine.nextEpoch();
  m_playing = false;
  Q_EMIT playerStateChanged();
}

/**
 * @brief Toggles between play and pause.
 */
void CSV::Player::toggle()
{
  if (m_playing)
    pause();
  else
    play();
}

/**
 * @brief Pauses at the indexing frontier and arms the auto-resume that fires when the next
 *        batch extends the indexed region.
 */
void CSV::Player::frontierPause()
{
  m_pausedAtFrontier = indexing();
  pause();
}

//--------------------------------------------------------------------------------------------------
// File operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts the user to select a CSV file to play back.
 */
void CSV::Player::openFile()
{
  auto& workspaceManager = Core::services().workspaceManager;
  auto* dialog           = new QFileDialog(qApp->activeWindow(),
                                 tr("Select CSV file"),
                                 workspaceManager.path("CSV"),
                                 tr("CSV files (*.csv)"));

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
 * @brief Closes the current CSV file: cancels + joins the indexer, then unmaps and resets
 *        playback state (teardown order is load-bearing -- the worker reads the mapping).
 */
void CSV::Player::closeFile()
{
  // code-verify off
  // A close reaching this while an inject is on the stack would unmap the file under the byte
  // views the builder still reads. Re-queue; the inject returns within one marshal.
  // code-verify on
  if (m_injecting) {
    QMetaObject::invokeMethod(this, [this] { closeFile(); }, Qt::QueuedConnection);
    return;
  }

  if (!isOpen())
    return;

  m_playing  = false;
  m_framePos = 0;
  m_engine.stopSeek();

  const bool joined = stopIndexing();

  if (m_mapped) {
    if (joined && m_csvFile)
      m_csvFile->unmap(reinterpret_cast<uchar*>(const_cast<char*>(m_mapped)));

    m_mapped = nullptr;
  }

  m_csvFile.reset();
  m_mappedSize = 0;
  m_dataOffset = 0;
  m_rowOffsets.clear();
  m_rowOffsets.squeeze();
  m_rowSeconds.clear();
  m_rowSeconds.squeeze();
  m_headerCells.clear();
  m_timestamp        = "--.--";
  m_timeScale        = 1.0;
  m_intervalSeconds  = 0.0;
  m_anchorMs         = 0;
  m_startSeconds     = -1.0;
  m_pausedAtFrontier = false;
  m_seekColumnByKey.clear();
  m_seekColumnByKey.squeeze();
  m_rows.reset();
  m_sources.clear();

  auto& frameBuilder = DataModel::pipelineModules().frameBuilder;
  frameBuilder.registerQuickPlotHeaders(QStringList());
  frameBuilder.setReplayColumnMap({});

  if (m_bus)
    m_bus->publishState<Core::Bus::ReplayPlayerStateChanged>(0, isOpen());

  Q_EMIT openChanged();
  Q_EMIT timestampChanged();
  Q_EMIT playerStateChanged();
  Q_EMIT indexingChanged();
}

/**
 * @brief Advances to the next CSV row, capped at the last indexed row.
 */
void CSV::Player::nextFrame()
{
  if (framePosition() < frameCount() - 1) {
    ++m_framePos;

    auto& dashboard = plotSink();
    dashboard.clearPlotData();

    int framesToLoad = dashboard.points();
    int startFrame   = std::max(1, m_framePos - framesToLoad);
    processFrameBatch(startFrame, m_framePos);

    updateData();
  }
}

/**
 * @brief Steps back to the previous CSV row, capped at the first row.
 */
void CSV::Player::previousFrame()
{
  if (framePosition() > 0) {
    --m_framePos;

    auto& dashboard = plotSink();
    dashboard.clearPlotData();

    int framesToLoad = dashboard.points();
    int startFrame   = std::max(1, m_framePos - framesToLoad);
    processFrameBatch(startFrame, m_framePos);

    updateData();
  }
}

/**
 * @brief Opens the CSV at filePath: maps the file, runs the foreground quick pass (header,
 *        timestamp detection, prompts), then hands indexing to the background worker so the
 *        UI is responsive immediately (spec 0022 R1).
 */
void CSV::Player::openFile(const QString& filePath)
{
  if (filePath.isEmpty())
    return;

  closeFile();

  if (!Replay::ensureLinkReleased(m_bus,
                                  tr("Device Connection Active"),
                                  tr("To use this feature, you must disconnect from the "
                                     "device. Do you want to proceed?"),
                                  Core::Prompt::Warning,
                                  qAppName()))
    return;

  m_csvFile = std::make_unique<QFile>(filePath);
  if (!m_csvFile->open(QIODevice::ReadOnly)) {
    Core::Prompt::showMessageBox(tr("Cannot read CSV file"),
                                 tr("Check file permissions and location"),
                                 Core::Prompt::Critical);
    closeFile();
    return;
  }

  m_mappedSize = m_csvFile->size();
  if (m_mappedSize <= 0) {
    Core::Prompt::showMessageBox(tr("Insufficient Data in CSV File"),
                                 tr("The CSV file must contain at least one data row to "
                                    "proceed. Check the file and try again."),
                                 Core::Prompt::Critical);
    closeFile();
    return;
  }

  m_mapped = reinterpret_cast<const char*>(m_csvFile->map(0, m_mappedSize));
  if (!m_mapped) {
    Core::Prompt::showMessageBox(tr("Cannot read CSV file"),
                                 tr("Check file permissions and location"),
                                 Core::Prompt::Critical);
    closeFile();
    return;
  }

  if (!runQuickPass()) {
    closeFile();
    return;
  }

  sendHeaderFrame();
  m_framePos = 0;
  startIndexing();

  if (m_bus)
    m_bus->publishState<Core::Bus::ReplayPlayerStateChanged>(0, isOpen());

  Q_EMIT openChanged();
  Q_EMIT playerStateChanged();
  Q_EMIT indexingChanged();
  updateData();
}

/**
 * @brief Foreground quick pass over the first rows only: skips a UTF-8 BOM, captures the
 *        header, detects the timestamp mode, and prompts when neither format matches. Any
 *        finite non-negative number is treated as an elapsed column; a negative value is
 *        rejected as having no usable time (PlayerLoaderWorker::extractTimestamp).
 */
bool CSV::Player::runQuickPass()
{
  SS_ASSERT(m_mapped != nullptr, return false);
  SS_ASSERT(m_mappedSize > 0, return false);

  qint64 pos = 0;
  if (m_mappedSize >= 3 && std::memcmp(m_mapped, "\xEF\xBB\xBF", 3) == 0)
    pos = 3;

  bool have_header = false;
  QByteArrayView header_row;
  QByteArrayView first_data_row;
  qint64 header_end = pos;

  for (qint64 guard = 0; guard <= m_mappedSize && pos <= m_mappedSize; ++guard) {
    const char* nl = static_cast<const char*>(
      std::memchr(m_mapped + pos, '\n', static_cast<size_t>(m_mappedSize - pos)));
    const qint64 end  = nl ? (nl - m_mapped) : m_mappedSize;
    const auto row    = QByteArrayView(m_mapped + pos, static_cast<qsizetype>(end - pos));
    const qint64 next = end + 1;
    const bool last   = (nl == nullptr);

    bool valid = false;
    if (!row.isEmpty() && row.size() <= kMaxCsvRowBytes) {
      m_rows.splitCells(row);
      const auto& cells = m_rows.cells();
      valid             = std::any_of(
        cells.cbegin(), cells.cend(), [](const QByteArrayView& c) { return !c.isEmpty(); });
    }

    if (valid && !have_header) {
      have_header = true;
      header_end  = next;
      header_row  = row;
    } else if (valid) {
      first_data_row = row;
      break;
    }

    if (last)
      break;

    pos = next;
  }

  if (!have_header || first_data_row.isEmpty()) {
    Core::Prompt::showMessageBox(tr("Insufficient Data in CSV File"),
                                 tr("The CSV file must contain at least one data row to "
                                    "proceed. Check the file and try again."),
                                 Core::Prompt::Critical);
    return false;
  }

  m_dataOffset = header_end;
  m_rows.setSeparator(sniffSeparator(header_row, first_data_row));

  m_rows.splitCells(header_row);
  m_headerCells.clear();
  for (const auto& cell : m_rows.cells())
    m_headerCells.append(QString::fromUtf8(cell));

  m_rows.splitCells(first_data_row);
  SS_ASSERT(!m_rows.cells().isEmpty(), return false);
  const QByteArrayView first_cell = m_rows.cells().first();

  bool is_number     = false;
  const double value = SerialStudio::toDouble(first_cell, &is_number);
  if (is_number && std::isfinite(value)) {
    m_rows.setTimestampMode(PlayerTimestampMode::Numeric, 0);
    const auto scale = timestampUnitScale(m_headerCells.first());
    m_timeScale      = scale ? *scale : promptTimestampUnitScale();
    return true;
  }

  qint64 anchor_ms = 0;
  if (!is_number && parseLegacyDateTimeMs(first_cell, anchor_ms)) {
    m_rows.setTimestampMode(PlayerTimestampMode::DateTime, 0);
    m_anchorMs = anchor_ms;
    return true;
  }

  return promptUserForDateTimeOrInterval(first_data_row);
}

/**
 * @brief Starts the background indexer for the current mapping on a fresh worker thread.
 */
void CSV::Player::startIndexing()
{
  SS_ASSERT(m_mapped != nullptr, return);
  SS_ASSERT(m_mappedSize > 0, return);

  auto request                 = std::make_shared<PlayerIndexRequest>();
  request->data                = m_mapped;
  request->size                = m_mappedSize;
  request->dataOffset          = m_dataOffset;
  request->timestampColumn     = m_rows.timestampColumn();
  request->intervalSeconds     = m_intervalSeconds;
  request->anchorMsSinceEpoch  = m_anchorMs;
  request->separator           = m_rows.separator();
  request->timeScale           = m_timeScale;
  request->mode                = m_rows.timestampMode();
  request->fileColumnSourceBit = m_sources.fileColumnSourceBit();

  m_pausedAtFrontier = false;
  m_indexer.start(request);
}

/**
 * @brief Cancels and joins the indexer thread; returns true when the join succeeded (only
 *        then may the caller unmap). A timed-out join detaches the worker, which may still
 *        be reading the mapping: the QFile that owns it goes with the detached thread, so
 *        the player must let go of it here rather than unmapping under a live reader.
 */
bool CSV::Player::stopIndexing()
{
  const bool joined = m_indexer.stop(m_csvFile.get());
  if (!joined)
    (void)m_csvFile.release();

  return joined;
}

/**
 * @brief Appends one indexed batch to the frontier: grows the timeline, refreshes progress,
 *        auto-resumes a frontier-paused playback and paints the first row once available.
 */
void CSV::Player::onIndexBatch(const CSV::PlayerIndexBatchPtr& batch)
{
  SS_ASSERT(batch != nullptr, return);

  if (!isOpen() || batch->generation != m_indexer.generation())
    return;

  const bool was_empty = m_rowOffsets.isEmpty();
  if (was_empty && !batch->rowOffsets.isEmpty() && batch->bytesIndexed > m_dataOffset) {
    const double avg_row =
      static_cast<double>(batch->bytesIndexed - m_dataOffset) / batch->rowOffsets.size();
    const double estimate   = static_cast<double>(m_mappedSize - m_dataOffset) / qMax(1.0, avg_row);
    const auto reserve_rows = static_cast<qsizetype>(estimate * 1.1) + 1024;
    m_rowOffsets.reserve(reserve_rows);
    m_rowSeconds.reserve(reserve_rows);
  }

  m_rowOffsets += batch->rowOffsets;
  m_rowSeconds += batch->rowSeconds;
  m_sources.appendRowSourceBits(batch->rowSourceBits);

  if (!m_playing && m_pausedAtFrontier
      && (frameCount() > m_framePos + 1 || (was_empty && frameCount() > 0))) {
    m_pausedAtFrontier = false;
    play();
  }

  Q_EMIT playerStateChanged();
  Q_EMIT indexingChanged();
  Q_EMIT timestampChanged();

  if (was_empty && frameCount() > 0)
    updateData();
}

/**
 * @brief Finalizes background indexing; an empty final index (no valid data row survived the
 *        scan) closes the player with the legacy insufficient-data message.
 */
void CSV::Player::onIndexFinished(bool ok, quint64 generation)
{
  if (!isOpen() || generation != m_indexer.generation())
    return;

  m_pausedAtFrontier = false;

  Q_EMIT indexingChanged();
  Q_EMIT playerStateChanged();
  Q_EMIT timestampChanged();

  if (ok && m_rowOffsets.size() >= kMaxIndexedRows)
    Core::Prompt::showMessageBox(
      tr("CSV Row Limit Reached"),
      tr("Playback is limited to %L1 rows; the rest of the file was not indexed.")
        .arg(kMaxIndexedRows),
      Core::Prompt::Warning);

  if (ok && frameCount() <= 0) {
    Core::Prompt::showMessageBox(tr("Insufficient Data in CSV File"),
                                 tr("The CSV file must contain at least one data row to "
                                    "proceed. Check the file and try again."),
                                 Core::Prompt::Critical);
    closeFile();
  }
}

//--------------------------------------------------------------------------------------------------
// Progress & seeking
//--------------------------------------------------------------------------------------------------

/**
 * @brief Seeks playback to a normalized position (tape scrub, spec 0020): the position and
 *        timestamp update immediately, a coalescing timer live-fills the plots at ~30 Hz, and
 *        the settle timer runs the exact full-window rebuild once the slider rests. While the
 *        index is still growing the target clamps to the frontier (spec 0022 R2).
 */
void CSV::Player::setProgress(const double progress)
{
  SS_ASSERT_LOG(isOpen());

  const auto validProgress = std::clamp(progress, 0.0, 1.0);

  if (isPlaying())
    pause();

  if (frameCount() <= 0)
    return;

  const int newFramePos = qMin(frameCount() - 1, qCeil(frameCount() * validProgress));
  if (newFramePos == m_framePos)
    return;

  m_framePos = newFramePos;
  updateTimestampDisplay();
  m_engine.armSeek();
}

/**
 * @brief First row of the scrub window ending at @p target: walks back until the plot time
 *        range is covered (never fewer than points() rows), capped by the engine so
 *        dense recordings bound the per-tick cost.
 */
int CSV::Player::seekWindowStartRow(int target)
{
  SS_ASSERT(target >= 0, return 0);
  SS_ASSERT(target < frameCount(), return qMax(0, frameCount() - 1));

  auto& dashboard = plotSink();
  return DataModel::ReplayPlaybackEngine::seekWindowStartRow(
    target, dashboard.points(), dashboard.plotTimeRange(), [this](int row) {
      return rowSecondsSinceStart(row);
    });
}

/**
 * @brief One coalesced scrub tick: bulk-fills the plot rings from the trailing window ending
 *        at the cursor and injects the cursor row so scalar widgets track it. Without a seek
 *        column map the settle rebuild runs instead -- a bulk fill with an empty series map
 *        would wipe the rings and blank the plots for the whole drag.
 */
void CSV::Player::performSeekTick()
{
  if (!isOpen() || isPlaying() || frameCount() <= 0)
    return;

  SS_ASSERT(m_framePos >= 0, return);
  SS_ASSERT(m_framePos < frameCount(), m_framePos = frameCount() - 1);

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
void CSV::Player::performSeekSettle()
{
  if (!isOpen() || isPlaying() || frameCount() <= 0)
    return;

  SS_ASSERT(m_framePos >= 0, return);
  SS_ASSERT(m_framePos < frameCount(), m_framePos = frameCount() - 1);

  auto& dashboard = plotSink();
  dashboard.clearPlotData();

  const int window = qMin(dashboard.points(), m_framePos + 1);
  const int start  = qMax(0, m_framePos - window + 1);
  processFrameBatch(start, m_framePos);
  m_sources.resetLastSourceRows();
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
 * @brief Fills the seek-window times and per-(source, uid) numeric series straight from the
 *        mapped rows (one split per row, fast_float per cell -- no QString); times are forced
 *        non-decreasing so the bulk fill's grid stays monotonic.
 */
void CSV::Player::buildSeekWindow(int startRow,
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
    const double t = rowSecondsSinceStart(startRow + k);
    times[k]       = (t >= 0.0) ? t : ((k > 0) ? times[k - 1] : 0.0);
    if (k > 0)
      times[k] = qMax(times[k], times[k - 1]);
  }

  auto& dashboard  = plotSink();
  const auto pairs = dashboard.replaySeekSeries();

  struct SeriesFill {
    int column;
    QVector<double>* values;
  };

  series.reserve(pairs.size());

  QVarLengthArray<SeriesFill, 32> fills;
  for (const auto& pair : pairs) {
    const qint64 key = DataModel::replaySeekKey(pair.first, pair.second);
    const int column = m_seekColumnByKey.value(key, -1);
    if (column < 0)
      continue;

    auto& values = series[key];
    values.resize(n);
    fills.append({column, &values});
  }

  if (fills.isEmpty())
    return;

  constexpr int kSeekSampleBudget = 8192;
  const int step                  = qMax(1, n / kSeekSampleBudget);
  for (int k = 0; k < n; k += step) {
    const int row             = (k + step < n) ? (startRow + k) : endRow;
    const qsizetype cellCount = m_rows.splitDataCells(rawRow(row));
    for (const auto& fill : fills) {
      const double v    = seekCellValue(m_rows.dataSpans(), cellCount, fill.column);
      const int fillEnd = qMin(n, k + step);
      for (int j = k; j < fillEnd; ++j)
        (*fill.values)[j] = v;
    }
  }

  for (const auto& fill : fills)
    fillSeekGaps(*fill.values);
}

//--------------------------------------------------------------------------------------------------
// Data processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the timestamp display for the current frame position.
 */
void CSV::Player::updateTimestampDisplay()
{
  if (frameCount() <= 0 || m_framePos >= frameCount())
    return;

  const double sec = rowSecondsSinceStart(m_framePos);

  if (sec >= 0.0) {
    if (m_rows.timestampMode() == PlayerTimestampMode::Numeric)
      m_timestamp = DataModel::ReplayPlaybackEngine::formatTimestamp(sec);
    else
      m_timestamp = DataModel::ReplayPlaybackEngine::formatTimestamp(
        sec - ((m_startSeconds >= 0.0) ? m_startSeconds : 0.0));
  }

  else {
    const qsizetype column = (m_rows.timestampMode() == PlayerTimestampMode::DateTimeColumn)
                             ? m_rows.timestampColumn()
                             : 0;
    m_rows.splitCells(rawRow(m_framePos));
    if (column >= 0 && column < m_rows.cells().size())
      m_timestamp = QString::fromUtf8(m_rows.cells().at(column));
  }

  Q_EMIT timestampChanged();
}

/**
 * @brief Furthest indexed row already due at @p target seconds (bounded scan). Rows without
 *        a usable time count as due in numeric mode (legacy zero-delay advance) and stop the
 *        scan in the date/time modes (legacy pause-on-invalid pacing).
 */
int CSV::Player::catchUpTargetRow(double target) const
{
  constexpr int kCatchUpScanMax = 262144;

  int row         = m_framePos;
  const int last  = frameCount() - 1;
  double previous = rowSecondsSinceStart(row);
  for (int i = 0; i < kCatchUpScanMax && row < last; ++i) {
    const double sec = rowSecondsSinceStart(row + 1);
    if (m_rows.timestampMode() != PlayerTimestampMode::Numeric && sec < 0.0)
      break;

    if (sec >= 0.0 && sec > target)
      break;

    if (sec >= 0.0 && previous >= 0.0 && sec < previous)
      break;

    previous = sec;
    ++row;
  }

  return row;
}

/**
 * @brief Recomputes msUntilNext for the current m_framePos, mirroring the legacy per-mode
 *        pacing (numeric clamps at 0; date/time pauses on an unusable next row). Pauses --
 *        frontier-aware -- and returns false at the end of the indexed region.
 */
bool CSV::Player::recomputeMsUntilNext(qint64& msUntilNext)
{
  constexpr double kMaxDelayMs = 86'400'000.0;

  const int next = m_framePos + 1;
  if (next >= frameCount()) {
    frontierPause();
    return false;
  }

  const double start   = (m_startSeconds >= 0.0) ? m_startSeconds : 0.0;
  const double target  = start + (m_elapsedTimer.elapsed() * kCsvInvMs);
  const double nextSec = rowSecondsSinceStart(next);
  const double deltaMs = (nextSec - target) * 1000.0;

  if (m_rows.timestampMode() == PlayerTimestampMode::Numeric) {
    msUntilNext =
      std::isfinite(deltaMs) ? static_cast<qint64>(std::clamp(deltaMs, 0.0, kMaxDelayMs)) : 0;
    return true;
  }

  if (nextSec < 0.0) {
    pause();
    return false;
  }

  msUntilNext = std::isfinite(deltaMs)
                ? static_cast<qint64>(std::clamp(deltaMs, -kMaxDelayMs, kMaxDelayMs))
                : 0;
  return true;
}

/**
 * @brief Processes current frame and schedules next frame for playback.
 */
void CSV::Player::updateData()
{
  SS_ASSERT(m_framePos >= 0, return);

  if (!isOpen())
    return;

  updateTimestampDisplay();
  if (!isPlaying())
    return;

  if (frameCount() <= 0) {
    frontierPause();
    return;
  }

  injectRow(framePosition());

  if (framePosition() >= frameCount() - 1) {
    frontierPause();
    return;
  }

  qint64 msUntilNext = 0;
  if (!recomputeMsUntilNext(msUntilNext))
    return;

  if (msUntilNext <= 0) {
    constexpr qint64 kCatchUpBudgetMs = 20;
    constexpr int kCatchUpMaxInjects  = 512;
    const QDeadlineTimer budget(kCatchUpBudgetMs);

    const double start  = (m_startSeconds >= 0.0) ? m_startSeconds : 0.0;
    const double target = start + (m_elapsedTimer.elapsed() * kCsvInvMs);
    const int targetRow = catchUpTargetRow(target);
    const int stride    = qMax(1, (targetRow - m_framePos) / kCatchUpMaxInjects);

    for (int processed = 0;
         processed < kCatchUpMaxInjects && m_framePos < targetRow && !budget.hasExpired();
         ++processed) {
      m_framePos = qMin(targetRow, m_framePos + stride);
      injectRow(m_framePos);
    }

    backfillSparseSources();

    if (stride > 2 && !m_seekColumnByKey.isEmpty() && m_engine.catchUpFillDue()) {
      auto& dashboard = plotSink();
      QVector<double> times;
      QHash<qint64, QVector<double>> series;
      buildSeekWindow(seekWindowStartRow(m_framePos), m_framePos, times, series);
      dashboard.bulkLoadPlotWindow(times, series);
    }

    updateTimestampDisplay();
    reanchorOnBackwardsRow();

    if (!recomputeMsUntilNext(msUntilNext))
      return;

    if (m_framePos < frameCount() - 1) {
      const quint64 epoch = m_engine.epoch();
      QTimer::singleShot(qMax(0LL, msUntilNext), Qt::PreciseTimer, this, [this, epoch] {
        if (isOpen() && isPlaying() && m_engine.isCurrentEpoch(epoch)) {
          ++m_framePos;
          updateData();
        }
      });
    } else
      frontierPause();
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
 * @brief Restarts the playback clock on the next row when the file's timestamps step BACKWARDS --
 *        a millis wrap or two logs concatenated. Without it every later row reads as "already due"
 *        and catch-up fast-forwards the whole file to EOF at 512 injects per 20 ms (B9).
 */
void CSV::Player::reanchorOnBackwardsRow()
{
  if (m_framePos < 0 || m_framePos >= frameCount() - 1)
    return;

  const double current = rowSecondsSinceStart(m_framePos);
  const double next    = rowSecondsSinceStart(m_framePos + 1);
  if (current < 0.0 || next < 0.0 || next >= current)
    return;

  m_startSeconds = next;
  m_elapsedTimer.start();
  anchorSteadyBase(m_framePos);
}

/**
 * @brief Synchronously injects frames in [startFrame, endFrame] for scrollback.
 */
void CSV::Player::processFrameBatch(int startFrame, int endFrame)
{
  SS_ASSERT(startFrame >= 0, return);
  SS_ASSERT(startFrame <= endFrame, return);

  if (!isOpen() || endFrame >= frameCount())
    return;

  anchorSteadyBase(startFrame);
  for (int i = startFrame; i <= endFrame; ++i)
    injectRow(i);
}

/**
 * @brief Registers CSV column names with Quick Plot (excluding the timestamp column) or, in
 *        project mode, installs the replay layout.
 */
void CSV::Player::sendHeaderFrame()
{
  const bool interval = (m_rows.timestampMode() == PlayerTimestampMode::Interval);
  if (m_headerCells.isEmpty() || (!interval && m_headerCells.size() <= 1))
    return;

  auto& appState = DataModel::pipelineModules().appState;
  if (appState.operationMode() == SerialStudio::ProjectFile) {
    buildReplayLayout();
    if (m_sources.multiSource())
      return;
  }

  QStringList headers;
  if (interval)
    headers = m_headerCells;
  else
    headers = m_headerCells.mid(1);

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

/**
 * @brief Asks for the numeric timestamp unit when the header names none (spec 0048 R7);
 *        seconds is preselected and cancel keeps it, so Enter/Escape preserve the legacy
 *        plain-seconds reading.
 */
double CSV::Player::promptTimestampUnitScale()
{
  SS_ASSERT_LOG(m_rows.timestampMode() == PlayerTimestampMode::Numeric);
  SS_ASSERT_LOG(!m_headerCells.isEmpty());

  if (nonInteractive())
    return 1.0;

  QStringList options;
  options << tr("Seconds (s)") << tr("Milliseconds (ms)") << tr("Microseconds (us)");

  bool ok              = false;
  const QString choice = QInputDialog::getItem(nullptr,
                                               tr("Timestamp Units"),
                                               tr("The timestamp column does not declare a "
                                                  "unit. How should it be interpreted?"),
                                               options,
                                               0,
                                               false,
                                               &ok);
  if (!ok)
    return 1.0;

  if (choice == options.at(1))
    return 1e-3;

  if (choice == options.at(2))
    return 1e-6;

  return 1.0;
}

/**
 * @brief Prompts the user to pick a date/time column or a manual row interval; configures the
 *        virtual timestamp mode instead of rewriting rows (spec 0022).
 */
bool CSV::Player::promptUserForDateTimeOrInterval(QByteArrayView firstDataRow)
{
  if (m_headerCells.isEmpty()) {
    Core::Prompt::showMessageBox(tr("Invalid CSV"),
                                 tr("The CSV file does not contain any data or headers."),
                                 Core::Prompt::Critical);
    return false;
  }

  if (nonInteractive()) {
    m_rows.setTimestampMode(PlayerTimestampMode::Interval, 0);
    m_intervalSeconds = kDefaultIntervalMs * kCsvInvMs;
    return true;
  }

  bool ok;
  QStringList options;
  options << tr("Select a date/time column") << tr("Set interval manually");
  QString choice = QInputDialog::getItem(nullptr,
                                         tr("CSV Date/Time Selection"),
                                         tr("Choose how to handle the date/time data:"),
                                         options,
                                         0,
                                         false,
                                         &ok);

  if (!ok)
    return false;

  if (choice == tr("Set interval manually")) {
    const auto interval =
      QInputDialog::getInt(nullptr,
                           tr("Set Interval"),
                           tr("Please enter the interval between rows in milliseconds:"),
                           kDefaultIntervalMs,
                           1,
                           1000000,
                           1,
                           &ok);

    if (ok) {
      m_rows.setTimestampMode(PlayerTimestampMode::Interval, 0);
      m_intervalSeconds = interval * kCsvInvMs;
      return true;
    }
  }

  else {
    const auto column =
      QInputDialog::getItem(nullptr,
                            tr("Select Date/Time Column"),
                            tr("Please select the column that contains the date/time data:"),
                            m_headerCells,
                            0,
                            false,
                            &ok);

    if (ok) {
      const int columnIndex = m_headerCells.indexOf(column);
      if (columnIndex == -1) {
        Core::Prompt::showMessageBox(
          tr("Invalid Selection"), tr("The selected column is not valid."), Core::Prompt::Critical);
        return false;
      }

      m_rows.setTimestampMode(PlayerTimestampMode::DateTimeColumn, columnIndex);

      m_rows.splitCells(firstDataRow);
      const auto& cells = m_rows.cells();
      qint64 anchor_ms  = 0;
      if (columnIndex < cells.size() && parseLegacyDateTimeMs(cells.at(columnIndex), anchor_ms))
        m_anchorMs = anchor_ms;
      else
        m_anchorMs = QDateTime::currentDateTime().toMSecsSinceEpoch();

      return true;
    }
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Row access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the raw bytes of one indexed row (newline excluded) as a view into the map.
 */
QByteArrayView CSV::Player::rawRow(int row) const
{
  if (row < 0 || row >= frameCount())
    return {};

  const qint64 begin = static_cast<qint64>(m_rowOffsets[row]);
  const char* nl     = static_cast<const char*>(
    std::memchr(m_mapped + begin, '\n', static_cast<size_t>(m_mappedSize - begin)));
  const qint64 end = nl ? (nl - m_mapped) : m_mappedSize;

  return QByteArrayView(m_mapped + begin, static_cast<qsizetype>(end - begin));
}

//--------------------------------------------------------------------------------------------------
// Multi-source playback helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the replay column layout from the export schema (uniqueId-ordered, virtual
 *        datasets included) and installs the per-source FrameBuilder lookup map.
 */
void CSV::Player::buildReplayLayout()
{
  m_seekColumnByKey.clear();

  DataModel::Frame frame;
  auto& projectModel = DataModel::pipelineModules().projectModel;
  frame.groups       = projectModel.groups();
  frame.sources      = projectModel.sources();
  const auto schema  = DataModel::buildExportSchema(frame);
  const int colCount = static_cast<int>(schema.columns.size());

  std::vector<CSV::ReplayColumnRef> columns;
  columns.reserve(schema.columns.size());
  for (int i = 0; i < colCount; ++i) {
    const auto& col = schema.columns[static_cast<size_t>(i)];
    columns.push_back({col.uniqueId, col.sourceId});
    m_seekColumnByKey.insert(DataModel::replaySeekKey(col.sourceId, col.uniqueId), i);
  }

  auto replay = m_sources.build(columns, static_cast<int>(m_headerCells.size()), [this](int i) {
    return m_rows.dataColumnToFileColumn(i);
  });

  auto& frameBuilder = DataModel::pipelineModules().frameBuilder;
  frameBuilder.setReplayColumnMap(std::move(replay));
}

/**
 * @brief Injects @p row for @p sourceId only: the backfill path updates one stale source from
 *        its own latest present row without re-publishing every other source at that instant.
 */
void CSV::Player::injectSourceRow(int row, int sourceId)
{
  SS_ASSERT(row >= 0, return);
  SS_ASSERT(row < frameCount(), return);

  if (m_injecting)
    return;

  const QScopedValueRollback<bool> reentry_guard(m_injecting, true);
  const qsizetype count = m_rows.splitDataCells(rawRow(row));
  if (count <= 0)
    return;

  const auto& columnsBySource = m_sources.sourceColumnsByIndex();
  const auto it               = columnsBySource.constFind(sourceId);
  if (it == columnsBySource.constEnd())
    return;

  auto& frameBuilder      = DataModel::pipelineModules().frameBuilder;
  const auto timestamp    = rowSteadyTimestamp(row);
  const auto* spans       = m_rows.dataSpans();
  const auto& orderedCols = it.value();
  QVarLengthArray<QByteArrayView, 64> cells;
  cells.reserve(orderedCols.size());
  for (int col : orderedCols)
    cells.append((col >= 0 && col < count) ? spans[col] : QByteArrayView());

  frameBuilder.replayChannelSpans(sourceId, cells.constData(), cells.size(), timestamp);
}

/**
 * @brief Brings every source the strided catch-up skipped up to its latest present row at or
 *        before the playhead (spec 0064): a sparse recording holds a slow source's cells on a
 *        handful of rows per second, so row-strided sampling almost never lands on one and its
 *        widgets would otherwise freeze for the whole replay.
 */
void CSV::Player::backfillSparseSources()
{
  SS_ASSERT_LOG(m_framePos >= 0);

  for (const auto& stale : m_sources.staleSources(m_framePos))
    injectSourceRow(stale.row, stale.sourceId);
}

/**
 * @brief Seconds-since-recording-start for @p row from the index; -1 when the row carries no
 *        usable time. Only ever consumed as deltas, so the anchor is arbitrary.
 */
double CSV::Player::rowSecondsSinceStart(int row) const
{
  SS_ASSERT(row >= 0, return -1.0);

  if (row < m_rowSeconds.size())
    return m_rowSeconds[row];

  return -1.0;
}

/**
 * @brief Anchors the steady-clock base used to stamp replayed rows with recorded deltas.
 */
void CSV::Player::anchorSteadyBase(int row)
{
  SS_ASSERT_LOG(row >= 0);

  m_engine.anchorSteadyBase(rowSecondsSinceStart(row));
}

/**
 * @brief Steady timestamp for @p row: the anchored base advanced by the recorded delta, so the
 *        recording -- not the wall clock -- owns replay time. Rows without a usable time fall
 *        back to now().
 */
std::chrono::steady_clock::time_point CSV::Player::rowSteadyTimestamp(int row)
{
  SS_ASSERT_LOG(row >= 0);

  const double seconds = rowSecondsSinceStart(row);
  if (seconds < 0.0) [[unlikely]]
    return std::chrono::steady_clock::now();

  return m_engine.steadyTimestampFor(seconds);
}

/**
 * @brief Replays one indexed row through the FrameBuilder replay span lane (spec 0022): cell
 *        views from the mapped bytes go straight in with the recorded timestamp -- no QString
 *        cells, no byte round-trip. QuickPlot mode keeps the byte path.
 */
void CSV::Player::injectRow(int row)
{
  SS_ASSERT(row >= 0, return);
  SS_ASSERT(row < frameCount(), return);

  // code-verify off
  // The replay marshal is a plain BlockingQueuedConnection: it does NOT run this thread's event
  // loop (dataflow.md). The latch is still what makes closeFile() re-queue instead of unmapping
  // the file under byte views the builder is reading, so it is a state flag, not a loop guard.
  // code-verify on
  if (m_injecting)
    return;

  const QScopedValueRollback<bool> reentry_guard(m_injecting, true);

  auto& appState = DataModel::pipelineModules().appState;
  if (appState.operationMode() != SerialStudio::ProjectFile) {
    injectFrame(m_rows.quickPlotPayload(rawRow(row)));
    return;
  }

  const qsizetype count = m_rows.splitDataCells(rawRow(row));
  if (count <= 0) [[unlikely]]
    return;

  auto& frameBuilder   = DataModel::pipelineModules().frameBuilder;
  const auto timestamp = rowSteadyTimestamp(row);
  const auto* spans    = m_rows.dataSpans();

  if (!m_sources.multiSource()) {
    frameBuilder.replayChannelSpans(0, spans, count, timestamp);
    return;
  }

  const auto& columnsBySource = m_sources.sourceColumnsByIndex();
  for (auto it = columnsBySource.constBegin(); it != columnsBySource.constEnd(); ++it) {
    const auto& orderedCols = it.value();
    QVarLengthArray<QByteArrayView, 64> cells;
    cells.reserve(orderedCols.size());
    bool present = false;
    for (int col : orderedCols) {
      const QByteArrayView cell = (col >= 0 && col < count) ? spans[col] : QByteArrayView();
      present                   = present || !cell.isEmpty();
      cells.append(cell);
    }

    if (present)
      frameBuilder.replayChannelSpans(it.key(), cells.constData(), cells.size(), timestamp);
  }
}

/**
 * @brief Injects a CSV frame, splitting per source when in multi-source mode.
 */
void CSV::Player::injectFrame(const QByteArray& frame)
{
  if (frame.isEmpty())
    return;

  if (!m_sources.multiSource()) {
    auto& connectionManager = payloadInjector();
    connectionManager.processPayload(frame);
    return;
  }

  const QString row = QString::fromUtf8(frame).trimmed();
  const auto fields = DataModel::splitReplayRow(row);

  QMap<int, QStringList> sourceFields;
  QSet<int> sourcesPresent;
  const auto& columnsBySource = m_sources.sourceColumnsByIndex();
  for (auto it = columnsBySource.constBegin(); it != columnsBySource.constEnd(); ++it) {
    const int srcId         = it.key();
    const auto& orderedCols = it.value();
    QStringList orderedCells;
    orderedCells.reserve(orderedCols.size());
    for (int col : orderedCols) {
      const QString cell = (col >= 0 && col < fields.size()) ? fields[col] : QString();
      orderedCells.append(cell);
      if (!cell.isEmpty())
        sourcesPresent.insert(srcId);
    }
    sourceFields.insert(srcId, std::move(orderedCells));
  }

  QMap<int, QByteArray> sourcePayloads;
  for (auto it = sourceFields.constBegin(); it != sourceFields.constEnd(); ++it) {
    if (!sourcesPresent.contains(it.key()))
      continue;

    sourcePayloads[it.key()] = DataModel::joinReplayRow(it.value()) + '\n';
  }

  if (sourcePayloads.isEmpty())
    return;

  auto& connectionManager = payloadInjector();
  connectionManager.processMultiSourcePayload(frame, sourcePayloads);
}

//--------------------------------------------------------------------------------------------------
// Event handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Captures key events and routes playback shortcuts to handleKeyPress.
 */
bool CSV::Player::eventFilter(QObject* obj, QEvent* event)
{
  if (isOpen() && event->type() == QEvent::KeyPress) {
    auto* keyEvent = static_cast<QKeyEvent*>(event);
    if (!DataModel::ReplayPlaybackEngine::playbackKeyIsClaimed(keyEvent->key()))
      return handleKeyPress(keyEvent);
  }

  return QObject::eventFilter(obj, event);
}

/**
 * @brief Maps media and arrow keys to playback actions.
 */
bool CSV::Player::handleKeyPress(QKeyEvent* keyEvent)
{
  bool handled;
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
      handled = false;
      break;
  }

  return handled;
}

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
#include <optional>
#include <QApplication>
#include <QDateTime>
#include <QDeadlineTimer>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QInputDialog>
#include <QScopedValueRollback>
#include <QSet>
#include <QTimer>
#include <QtMath>
#include <unordered_map>

#include "AppState.h"
#include "DataModel/ExportSchema.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "SerialStudio.h"
#include "SSAssert.h"
#include "UI/Dashboard.h"

static constexpr double kCsvInvMs       = 1.0 / 1000.0;
static constexpr int kMaxSeekWindowRows = 262144;
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
 * @brief Formats fractional seconds as HH:MM:SS.mmm.
 */
[[nodiscard]] static QString formatTimestamp(double seconds)
{
  constexpr double kInvHour = 1.0 / 3600.0;
  constexpr double kInvMin  = 1.0 / 60.0;

  int hours   = static_cast<int>(seconds * kInvHour);
  int minutes = static_cast<int>((seconds - hours * 3600.0) * kInvMin);
  double secs = seconds - hours * 3600.0 - minutes * 60.0;

  return QString("%1:%2:%3")
    .arg(hours, 2, 10, QChar('0'))
    .arg(minutes, 2, 10, QChar('0'))
    .arg(secs, 6, 'f', 3, QChar('0'));
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

/**
 * @brief Index of the first top-level @p separator of a raw row (quote-aware, mirroring the
 *        replay splitter's machine), or -1 when the row has a single cell.
 */
[[nodiscard]] static qsizetype firstTopLevelSeparator(QByteArrayView row, char separator)
{
  bool in_quotes       = false;
  bool was_quoted      = false;
  bool only_space_seen = true;

  const qsizetype length = row.size();
  for (qsizetype i = 0; i < length; ++i) {
    const char c = row.at(i);

    if (in_quotes) {
      const bool escaped = c == '"' && i + 1 < length && row.at(i + 1) == '"';
      if (escaped) {
        ++i;
        continue;
      }

      if (c == '"')
        in_quotes = false;

      continue;
    }

    if (c == separator)
      return i;

    if (c == '"' && !was_quoted && only_space_seen) {
      in_quotes  = true;
      was_quoted = true;
      continue;
    }

    if (c != ' ' && c != '\t' && c != '\n' && c != '\v' && c != '\f' && c != '\r')
      only_space_seen = false;
  }

  return -1;
}

/**
 * @brief Top-level occurrences of @p separator in @p row using a cell-position-independent
 *        quote scanner (any double quote toggles quoted mode, "" escapes): unlike the RFC
 *        splitter's machine, quoted content never scores for ANY candidate, so a quoted cell
 *        cannot leak separators into the sniff of a differently-separated file.
 */
[[nodiscard]] static qsizetype topLevelSeparatorCount(QByteArrayView row, char separator)
{
  SS_ASSERT(separator != '"', return 0);
  SS_ASSERT_LOG(!row.isEmpty());

  bool in_quotes         = false;
  qsizetype count        = 0;
  const qsizetype length = row.size();
  for (qsizetype i = 0; i < length; ++i) {
    const char c = row.at(i);

    if (c == '"') {
      const bool escaped = in_quotes && i + 1 < length && row.at(i + 1) == '"';
      if (escaped) {
        ++i;
        continue;
      }

      in_quotes = !in_quotes;
      continue;
    }

    if (!in_quotes && c == separator)
      ++count;
  }

  return count;
}

/**
 * @brief Detects the row separator (spec 0048): candidates scored by summed top-level
 *        occurrence count over header + first data row, comma scored first and winning
 *        ties. A non-comma winner needs a data-row hit AND header count == data count,
 *        so text-cell separators ("1,a;b;c") never re-interpret a comma file.
 */
[[nodiscard]] static char sniffSeparator(QByteArrayView headerRow, QByteArrayView dataRow)
{
  SS_ASSERT(!headerRow.isEmpty(), return ',');
  SS_ASSERT(!dataRow.isEmpty(), return ',');

  constexpr char kCandidates[] = {',', ';', '\t', '|'};

  char best           = ',';
  qsizetype bestScore = -1;
  for (const char candidate : kCandidates) {
    const qsizetype data_count   = topLevelSeparatorCount(dataRow, candidate);
    const qsizetype header_count = topLevelSeparatorCount(headerRow, candidate);
    const qsizetype score        = header_count + data_count;

    const bool eligible = (candidate == ',') || (data_count >= 1 && header_count == data_count);
    if (eligible && score > bestScore) {
      best      = candidate;
      bestScore = score;
    }
  }

  return best;
}

/**
 * @brief Seconds-per-tick for a numeric timestamp header (spec 0048 R7): a bracketed unit
 *        ("time(ms)", "t [us]") or a "_unit" suffix ("time_ms") selects the scale; a
 *        header naming no recognizable unit returns nullopt so the caller can ask.
 */
[[nodiscard]] static std::optional<double> timestampUnitScale(const QString& header)
{
  const QString text = header.trimmed().toLower();
  SS_ASSERT(!text.isEmpty(), return std::nullopt);

  QString unit;
  const qsizetype paren   = text.lastIndexOf(QChar('('));
  const qsizetype bracket = text.lastIndexOf(QChar('['));
  if (paren >= 0 && text.endsWith(QChar(')')))
    unit = text.mid(paren + 1, text.size() - paren - 2).trimmed();
  else if (bracket >= 0 && text.endsWith(QChar(']')))
    unit = text.mid(bracket + 1, text.size() - bracket - 2).trimmed();
  else if (text.lastIndexOf(QChar('_')) >= 0)
    unit = text.mid(text.lastIndexOf(QChar('_')) + 1).trimmed();

  SS_ASSERT_LOG(unit.size() <= text.size());

  if (unit == QStringLiteral("ms") || unit == QStringLiteral("msec")
      || unit == QStringLiteral("millis") || unit == QStringLiteral("milliseconds"))
    return 1e-3;

  if (unit == QStringLiteral("us") || unit == QStringLiteral("\u00b5s")
      || unit == QStringLiteral("usec") || unit == QStringLiteral("microseconds"))
    return 1e-6;

  if (unit == QStringLiteral("ns") || unit == QStringLiteral("nsec")
      || unit == QStringLiteral("nanoseconds"))
    return 1e-9;

  if (unit == QStringLiteral("s") || unit == QStringLiteral("sec") || unit == QStringLiteral("secs")
      || unit == QStringLiteral("seconds"))
    return 1.0;

  return std::nullopt;
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
  , m_multiSource(false)
  , m_indexing(false)
  , m_pausedAtFrontier(false)
  , m_timestamp("")
  , m_mapped(nullptr)
  , m_mappedSize(0)
  , m_dataOffset(0)
  , m_bytesIndexed(0)
  , m_indexGeneration(0)
  , m_playbackEpoch(0)
  , m_tsMode(PlayerTimestampMode::Numeric)
  , m_timestampColumn(0)
  , m_separator(',')
  , m_timeScale(1.0)
  , m_intervalSeconds(0.0)
  , m_anchorMs(0)
  , m_startSeconds(-1.0)
  , m_steadyBaseRowSeconds(0.0)
  , m_loaderThread(nullptr)
  , m_loader(nullptr)
{
  qApp->installEventFilter(this);
  qRegisterMetaType<CSV::PlayerIndexRequestPtr>();
  qRegisterMetaType<CSV::PlayerIndexBatchPtr>();

  constexpr int kSeekTickMs   = 33;
  constexpr int kSeekSettleMs = 250;
  m_seekTimer.setSingleShot(true);
  m_seekTimer.setInterval(kSeekTickMs);
  m_settleTimer.setSingleShot(true);
  m_settleTimer.setInterval(kSeekSettleMs);
  connect(&m_seekTimer, &QTimer::timeout, this, &CSV::Player::performSeekTick);
  connect(&m_settleTimer, &QTimer::timeout, this, &CSV::Player::performSeekSettle);
}

/**
 * @brief Destructor - joins any in-flight indexer so the worker never outlives the mapping.
 */
CSV::Player::~Player()
{
  stopIndexing();
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
  return m_indexing;
}

/**
 * @brief Returns background-indexing progress in the range 0.0 to 1.0.
 */
double CSV::Player::indexProgress() const
{
  if (m_mappedSize <= 0)
    return 1.0;

  return std::clamp(static_cast<double>(m_bytesIndexed) / m_mappedSize, 0.0, 1.0);
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
    m_pausedAtFrontier = m_indexing;
    return;
  }

  if (m_framePos >= frameCount() - 1 && !m_indexing)
    m_framePos = 0;

  m_pausedAtFrontier = false;
  ++m_playbackEpoch;
  m_startSeconds = rowSecondsSinceStart(m_framePos);
  m_elapsedTimer.start();

  m_seekTimer.stop();
  m_settleTimer.stop();

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

  ++m_playbackEpoch;
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
  m_pausedAtFrontier = m_indexing;
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
  static auto& workspaceManager = Misc::WorkspaceManager::instance();
  auto* dialog                  = new QFileDialog(qApp->activeWindow(),
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
  // A close dispatched inside injectRow's nested event loop would unmap the file while the
  // builder still reads its byte views. Re-queue; the inject returns within one marshal.
  // code-verify on
  if (m_injecting) {
    QMetaObject::invokeMethod(this, [this] { closeFile(); }, Qt::QueuedConnection);
    return;
  }

  if (!isOpen())
    return;

  m_playing  = false;
  m_framePos = 0;
  m_seekTimer.stop();
  m_settleTimer.stop();

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
  m_rowSourceBits.clear();
  m_rowSourceBits.squeeze();
  m_fileColumnSourceBit.clear();
  m_bitSourceIds.clear();
  m_lastSourceRow.clear();
  m_timestamp        = "--.--";
  m_tsMode           = PlayerTimestampMode::Numeric;
  m_timestampColumn  = 0;
  m_separator        = ',';
  m_timeScale        = 1.0;
  m_intervalSeconds  = 0.0;
  m_anchorMs         = 0;
  m_startSeconds     = -1.0;
  m_bytesIndexed     = 0;
  m_multiSource      = false;
  m_pausedAtFrontier = false;
  m_seekColumnByKey.clear();
  m_seekColumnByKey.squeeze();
  m_sourceColumnsByIndex.clear();
  m_cells        = {};
  m_splitScratch = QByteArray();
  m_dataSpans    = {};

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  frameBuilder.registerQuickPlotHeaders(QStringList());
  frameBuilder.setReplayColumnMap({});

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

    static auto& dashboard = UI::Dashboard::instance();
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

    static auto& dashboard = UI::Dashboard::instance();
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

  m_csvFile = std::make_unique<QFile>(filePath);
  if (!m_csvFile->open(QIODevice::ReadOnly)) {
    Misc::Utilities::showMessageBox(
      tr("Cannot read CSV file"), tr("Check file permissions and location"), QMessageBox::Critical);
    closeFile();
    return;
  }

  m_mappedSize = m_csvFile->size();
  if (m_mappedSize <= 0) {
    Misc::Utilities::showMessageBox(tr("Insufficient Data in CSV File"),
                                    tr("The CSV file must contain at least one data row to "
                                       "proceed. Check the file and try again."),
                                    QMessageBox::Critical);
    closeFile();
    return;
  }

  m_mapped = reinterpret_cast<const char*>(m_csvFile->map(0, m_mappedSize));
  if (!m_mapped) {
    Misc::Utilities::showMessageBox(
      tr("Cannot read CSV file"), tr("Check file permissions and location"), QMessageBox::Critical);
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

  Q_EMIT openChanged();
  Q_EMIT playerStateChanged();
  Q_EMIT indexingChanged();
  updateData();
}

/**
 * @brief Foreground quick pass over the first rows only: skips a UTF-8 BOM, captures the
 *        header, detects the timestamp mode, and prompts when neither format matches. Any
 *        finite number is an elapsed column, negatives included -- multi-source recordings
 *        written before the exporter latched its origin start below zero.
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
      DataModel::splitReplayRowSpans(row, m_cells, m_splitScratch, m_separator);
      valid = std::any_of(
        m_cells.cbegin(), m_cells.cend(), [](const QByteArrayView& c) { return !c.isEmpty(); });
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
    Misc::Utilities::showMessageBox(tr("Insufficient Data in CSV File"),
                                    tr("The CSV file must contain at least one data row to "
                                       "proceed. Check the file and try again."),
                                    QMessageBox::Critical);
    return false;
  }

  m_dataOffset = header_end;
  m_separator  = sniffSeparator(header_row, first_data_row);

  DataModel::splitReplayRowSpans(header_row, m_cells, m_splitScratch, m_separator);
  m_headerCells.clear();
  for (const auto& cell : m_cells)
    m_headerCells.append(QString::fromUtf8(cell));

  DataModel::splitReplayRowSpans(first_data_row, m_cells, m_splitScratch, m_separator);
  SS_ASSERT(!m_cells.isEmpty(), return false);
  const QByteArrayView first_cell = m_cells.first();

  bool is_number     = false;
  const double value = SerialStudio::toDouble(first_cell, &is_number);
  if (is_number && std::isfinite(value)) {
    m_tsMode         = PlayerTimestampMode::Numeric;
    const auto scale = timestampUnitScale(m_headerCells.first());
    m_timeScale      = scale ? *scale : promptTimestampUnitScale();
    return true;
  }

  qint64 anchor_ms = 0;
  if (!is_number && parseLegacyDateTimeMs(first_cell, anchor_ms)) {
    m_tsMode   = PlayerTimestampMode::DateTime;
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
  SS_ASSERT(m_loaderThread == nullptr, return);

  ++m_indexGeneration;
  m_loaderThread = new QThread(this);
  m_loaderThread->setObjectName(QStringLiteral("CSV::PlayerLoader"));
  m_loader = new PlayerLoaderWorker();
  m_loader->moveToThread(m_loaderThread);

  connect(m_loader,
          &PlayerLoaderWorker::batchReady,
          this,
          &CSV::Player::onIndexBatch,
          Qt::QueuedConnection);
  connect(m_loader,
          &PlayerLoaderWorker::finished,
          this,
          &CSV::Player::onIndexFinished,
          Qt::QueuedConnection);

  m_loaderThread->start();

  auto request                 = std::make_shared<PlayerIndexRequest>();
  request->data                = m_mapped;
  request->size                = m_mappedSize;
  request->dataOffset          = m_dataOffset;
  request->timestampColumn     = m_timestampColumn;
  request->intervalSeconds     = m_intervalSeconds;
  request->anchorMsSinceEpoch  = m_anchorMs;
  request->generation          = m_indexGeneration;
  request->separator           = m_separator;
  request->timeScale           = m_timeScale;
  request->mode                = m_tsMode;
  request->fileColumnSourceBit = m_fileColumnSourceBit;

  m_indexing         = true;
  m_pausedAtFrontier = false;
  m_bytesIndexed     = m_dataOffset;

  auto* loader = m_loader;
  QMetaObject::invokeMethod(
    loader, [loader, request]() { loader->indexFile(request); }, Qt::QueuedConnection);
}

/**
 * @brief Cancels and joins the indexer thread; returns true when the join succeeded (only
 *        then may the caller unmap -- a detached runaway worker may still read the mapping,
 *        so the timeout branch reparents the thread for self-cleanup and reports false).
 */
bool CSV::Player::stopIndexing()
{
  if (!m_loaderThread)
    return true;

  constexpr int kJoinTimeoutMs = 5000;

  if (m_loader)
    m_loader->requestCancel();

  m_loaderThread->quit();
  const bool joined = m_loaderThread->wait(kJoinTimeoutMs);
  if (joined) {
    delete m_loader;
    delete m_loaderThread;
  } else {
    qWarning() << "[CSV::Player] Indexer thread did not stop in time; detaching it.";
    disconnect(m_loader, nullptr, this, nullptr);
    m_loaderThread->setParent(nullptr);
    connect(m_loaderThread, &QThread::finished, m_loader, &QObject::deleteLater);
    connect(m_loaderThread, &QThread::finished, m_loaderThread, &QObject::deleteLater);

    // code-verify off
    // The detached worker is still reading the mapping, and the QFile owns it: closing or
    // destroying the file here unmaps memory under a live reader. Hand ownership to the thread.
    // code-verify on
    if (m_csvFile) {
      auto* file = m_csvFile.release();
      connect(m_loaderThread, &QThread::finished, file, &QObject::deleteLater);
    }
  }

  m_loader       = nullptr;
  m_loaderThread = nullptr;
  m_indexing     = false;
  return joined;
}

/**
 * @brief Appends one indexed batch to the frontier: grows the timeline, refreshes progress,
 *        auto-resumes a frontier-paused playback and paints the first row once available.
 */
void CSV::Player::onIndexBatch(const CSV::PlayerIndexBatchPtr& batch)
{
  SS_ASSERT(batch != nullptr, return);

  if (!isOpen() || batch->generation != m_indexGeneration)
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

  m_rowOffsets    += batch->rowOffsets;
  m_rowSeconds    += batch->rowSeconds;
  m_rowSourceBits += batch->rowSourceBits;
  m_bytesIndexed   = batch->bytesIndexed;

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
  if (!isOpen() || generation != m_indexGeneration)
    return;

  m_indexing         = false;
  m_pausedAtFrontier = false;
  if (ok)
    m_bytesIndexed = m_mappedSize;

  Q_EMIT indexingChanged();
  Q_EMIT playerStateChanged();
  Q_EMIT timestampChanged();

  if (ok && m_rowOffsets.size() >= kMaxIndexedRows)
    Misc::Utilities::showMessageBox(
      tr("CSV Row Limit Reached"),
      tr("Playback is limited to %L1 rows; the rest of the file was not indexed.")
        .arg(kMaxIndexedRows),
      QMessageBox::Warning);

  if (ok && frameCount() <= 0) {
    Misc::Utilities::showMessageBox(tr("Insufficient Data in CSV File"),
                                    tr("The CSV file must contain at least one data row to "
                                       "proceed. Check the file and try again."),
                                    QMessageBox::Critical);
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

  if (!m_seekTimer.isActive())
    m_seekTimer.start();

  m_settleTimer.start();
}

/**
 * @brief First row of the scrub window ending at @p target: walks back until the plot time
 *        range is covered (never fewer than points() rows), capped at kMaxSeekWindowRows so
 *        dense recordings bound the per-tick cost.
 */
int CSV::Player::seekWindowStartRow(int target)
{
  SS_ASSERT(target >= 0, return 0);
  SS_ASSERT(target < frameCount(), return qMax(0, frameCount() - 1));

  static auto& dashboard = UI::Dashboard::instance();
  const double range     = dashboard.plotTimeRange();
  const double targetSec = rowSecondsSinceStart(target);

  const int minStart = qMax(0, target - qMax(1, dashboard.points()) + 1);
  const int capStart = qMax(0, target - kMaxSeekWindowRows + 1);

  int start = minStart;
  for (int i = 0; i < kMaxSeekWindowRows && targetSec >= 0.0 && start > capStart; ++i) {
    const double sec = rowSecondsSinceStart(start - 1);
    if (sec < 0.0 || targetSec - sec > range)
      break;

    --start;
  }

  return start;
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

  static auto& dashboard = UI::Dashboard::instance();
  const int target       = m_framePos;
  const int start        = seekWindowStartRow(target);

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

  static auto& dashboard = UI::Dashboard::instance();
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

  static auto& dashboard = UI::Dashboard::instance();
  const auto pairs       = dashboard.replaySeekSeries();

  struct SeriesFill {
    int column;
    QVector<double>* values;
  };

  series.reserve(pairs.size());

  QVarLengthArray<SeriesFill, 32> fills;
  for (const auto& pair : pairs) {
    const qint64 key = UI::Dashboard::replaySeekKey(pair.first, pair.second);
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
    const qsizetype cellCount = splitDataCells(row);
    for (const auto& fill : fills) {
      const double v    = seekCellValue(m_dataSpans.constData(), cellCount, fill.column);
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
    if (m_tsMode == PlayerTimestampMode::Numeric)
      m_timestamp = formatTimestamp(sec);
    else
      m_timestamp = formatTimestamp(sec - ((m_startSeconds >= 0.0) ? m_startSeconds : 0.0));
  }

  else {
    const qsizetype column =
      (m_tsMode == PlayerTimestampMode::DateTimeColumn) ? m_timestampColumn : 0;
    DataModel::splitReplayRowSpans(rawRow(m_framePos), m_cells, m_splitScratch, m_separator);
    if (column >= 0 && column < m_cells.size())
      m_timestamp = QString::fromUtf8(m_cells.at(column));
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

  int row        = m_framePos;
  const int last = frameCount() - 1;
  for (int i = 0; i < kCatchUpScanMax && row < last; ++i) {
    const double sec = rowSecondsSinceStart(row + 1);
    if (m_tsMode != PlayerTimestampMode::Numeric && sec < 0.0)
      break;

    if (sec >= 0.0 && sec > target)
      break;

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

  if (m_tsMode == PlayerTimestampMode::Numeric) {
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

    constexpr qint64 kCatchUpFillMs = 250;
    if (stride > 2 && !m_seekColumnByKey.isEmpty()
        && (!m_catchUpFillTimer.isValid() || m_catchUpFillTimer.elapsed() >= kCatchUpFillMs)) {
      static auto& dashboard = UI::Dashboard::instance();
      QVector<double> times;
      QHash<qint64, QVector<double>> series;
      buildSeekWindow(seekWindowStartRow(m_framePos), m_framePos, times, series);
      dashboard.bulkLoadPlotWindow(times, series);
      m_catchUpFillTimer.restart();
    }

    updateTimestampDisplay();

    if (!recomputeMsUntilNext(msUntilNext))
      return;

    if (m_framePos < frameCount() - 1) {
      const quint64 epoch = m_playbackEpoch;
      QTimer::singleShot(qMax(0LL, msUntilNext), Qt::PreciseTimer, this, [this, epoch] {
        if (isOpen() && isPlaying() && epoch == m_playbackEpoch) {
          ++m_framePos;
          updateData();
        }
      });
    } else
      frontierPause();
  }

  else {
    const quint64 epoch = m_playbackEpoch;
    QTimer::singleShot(msUntilNext, Qt::PreciseTimer, this, [this, epoch] {
      if (!isOpen() || !isPlaying() || epoch != m_playbackEpoch)
        return;

      ++m_framePos;
      updateData();
    });
  }
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
  const bool interval = (m_tsMode == PlayerTimestampMode::Interval);
  if (m_headerCells.isEmpty() || (!interval && m_headerCells.size() <= 1))
    return;

  static auto& appState = AppState::instance();
  if (appState.operationMode() == SerialStudio::ProjectFile) {
    buildReplayLayout();
    if (m_multiSource)
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
      m_seekColumnByKey.insert(
        UI::Dashboard::replaySeekKey(0, DataModel::dataset_unique_id(0, 0, i)), i);
      m_seekColumnByKey.insert(
        UI::Dashboard::replaySeekKey(0, DataModel::dataset_unique_id(0, 1, i)), i);
    }
  }

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  frameBuilder.registerQuickPlotHeaders(headers);
}

/**
 * @brief Asks for the numeric timestamp unit when the header names none (spec 0048 R7);
 *        seconds is preselected and cancel keeps it, so Enter/Escape preserve the legacy
 *        plain-seconds reading.
 */
double CSV::Player::promptTimestampUnitScale()
{
  SS_ASSERT_LOG(m_tsMode == PlayerTimestampMode::Numeric);
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
    Misc::Utilities::showMessageBox(tr("Invalid CSV"),
                                    tr("The CSV file does not contain any data or headers."),
                                    QMessageBox::Critical);
    return false;
  }

  if (nonInteractive()) {
    m_tsMode          = PlayerTimestampMode::Interval;
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
      m_tsMode          = PlayerTimestampMode::Interval;
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
        Misc::Utilities::showMessageBox(
          tr("Invalid Selection"), tr("The selected column is not valid."), QMessageBox::Critical);
        return false;
      }

      m_tsMode          = PlayerTimestampMode::DateTimeColumn;
      m_timestampColumn = columnIndex;

      DataModel::splitReplayRowSpans(firstDataRow, m_cells, m_splitScratch, m_separator);
      qint64 anchor_ms = 0;
      if (columnIndex < m_cells.size() && parseLegacyDateTimeMs(m_cells.at(columnIndex), anchor_ms))
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

/**
 * @brief Splits @p row and fills m_dataSpans with its data cells (timestamp column excluded
 *        per the active mode); returns the data-cell count. Views stay valid until the next
 *        split or closeFile().
 */
qsizetype CSV::Player::splitDataCells(int row)
{
  DataModel::splitReplayRowSpans(rawRow(row), m_cells, m_splitScratch, m_separator);

  m_dataSpans.clear();
  switch (m_tsMode) {
    case PlayerTimestampMode::Interval:
      for (const auto& cell : m_cells)
        m_dataSpans.append(cell);

      break;

    case PlayerTimestampMode::DateTimeColumn:
      for (qsizetype i = 0; i < m_cells.size(); ++i)
        if (i != m_timestampColumn)
          m_dataSpans.append(m_cells.at(i));

      break;

    case PlayerTimestampMode::Numeric:
    case PlayerTimestampMode::DateTime:
      for (qsizetype i = 1; i < m_cells.size(); ++i)
        m_dataSpans.append(m_cells.at(i));

      break;
  }

  return m_dataSpans.size();
}

/**
 * @brief Builds the QuickPlot byte payload for @p row: the raw row minus the timestamp cell,
 *        sliced verbatim from the map where possible. Date-time-column mode and non-comma
 *        files (spec 0048) rebuild through the joiner, so injected payloads are always
 *        RFC-4180 comma rows for the downstream comma-only splitters.
 */
QByteArray CSV::Player::quickPlotPayload(int row)
{
  SS_ASSERT(row >= 0, return {});
  SS_ASSERT(row < frameCount(), return {});

  auto view = rawRow(row);
  if (view.endsWith('\r'))
    view.chop(1);

  if (m_separator != ',' || m_tsMode == PlayerTimestampMode::DateTimeColumn) {
    const qsizetype count = splitDataCells(row);
    QStringList cells;
    cells.reserve(count);
    for (qsizetype i = 0; i < count; ++i)
      cells.append(QString::fromUtf8(m_dataSpans.at(i)));

    QByteArray frame = DataModel::joinReplayRow(cells);
    frame.append('\n');
    return frame;
  }

  if (m_tsMode == PlayerTimestampMode::Interval) {
    QByteArray frame(view.constData(), view.size());
    frame.append('\n');
    return frame;
  }

  const qsizetype comma = firstTopLevelSeparator(view, m_separator);
  if (comma < 0)
    return QByteArray();

  QByteArray frame(view.constData() + comma + 1, view.size() - comma - 1);
  frame.append('\n');
  return frame;
}

//--------------------------------------------------------------------------------------------------
// Multi-source playback helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the replay column layout from the export schema (uniqueId-ordered, virtual
 * datasets included) and installs the per-source FrameBuilder lookup map.
 */
void CSV::Player::buildReplayLayout()
{
  m_seekColumnByKey.clear();
  m_sourceColumnsByIndex.clear();

  DataModel::Frame frame;
  static auto& projectModel = DataModel::ProjectModel::instance();
  frame.groups              = projectModel.groups();
  frame.sources             = projectModel.sources();
  const auto schema         = DataModel::buildExportSchema(frame);
  const int colCount        = static_cast<int>(schema.columns.size());

  QSet<int> sources;
  for (int i = 0; i < colCount; ++i) {
    const auto& col = schema.columns[static_cast<size_t>(i)];
    sources.insert(col.sourceId);
    m_seekColumnByKey.insert(UI::Dashboard::replaySeekKey(col.sourceId, col.uniqueId), i);
  }

  m_multiSource = sources.size() > 1;

  std::unordered_map<int, std::unordered_map<int, int>> replay;

  if (!m_multiSource) {
    for (int i = 0; i < colCount; ++i)
      replay[0][schema.columns[static_cast<size_t>(i)].uniqueId] = i;
  }

  else {
    std::unordered_map<int, int> nextLocal;
    for (int i = 0; i < colCount; ++i) {
      const auto& col = schema.columns[static_cast<size_t>(i)];
      m_sourceColumnsByIndex[col.sourceId].append(i);
      replay[col.sourceId][col.uniqueId] = nextLocal[col.sourceId]++;
    }
  }

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  frameBuilder.setReplayColumnMap(std::move(replay));

  m_bitSourceIds.clear();
  m_fileColumnSourceBit = QVector<quint8>(m_headerCells.size(), 0);
  for (int i = 0; i < colCount; ++i) {
    const int srcId = schema.columns[static_cast<size_t>(i)].sourceId;
    int bit         = static_cast<int>(m_bitSourceIds.indexOf(srcId));
    if (bit < 0 && m_bitSourceIds.size() < 8) {
      bit = m_bitSourceIds.size();
      m_bitSourceIds.append(srcId);
    }

    const int fileCol = dataColumnToFileColumn(i);
    if (bit >= 0 && fileCol >= 0 && fileCol < m_fileColumnSourceBit.size())
      m_fileColumnSourceBit[fileCol] = static_cast<quint8>(1u << bit);
  }

  m_lastSourceRow = QVector<int>(m_bitSourceIds.size(), -1);
}

/**
 * @brief File cell index of data column @p i, undoing the timestamp-column exclusion that
 *        splitDataCells applies for the active timestamp mode.
 */
int CSV::Player::dataColumnToFileColumn(int i) const
{
  SS_ASSERT(i >= 0, return -1);

  switch (m_tsMode) {
    case PlayerTimestampMode::Interval:
      return i;
    case PlayerTimestampMode::DateTimeColumn:
      return (i < m_timestampColumn) ? i : i + 1;
    case PlayerTimestampMode::Numeric:
    case PlayerTimestampMode::DateTime:
      return i + 1;
  }

  return -1;
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
  const qsizetype count = splitDataCells(row);
  if (count <= 0)
    return;

  const auto it = m_sourceColumnsByIndex.constFind(sourceId);
  if (it == m_sourceColumnsByIndex.constEnd())
    return;

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  const auto timestamp      = rowSteadyTimestamp(row);
  const auto& orderedCols   = it.value();
  QVarLengthArray<QByteArrayView, 64> cells;
  cells.reserve(orderedCols.size());
  for (int col : orderedCols)
    cells.append((col >= 0 && col < count) ? m_dataSpans.at(col) : QByteArrayView());

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

  m_steadyBase           = std::chrono::steady_clock::now();
  const double seconds   = rowSecondsSinceStart(row);
  m_steadyBaseRowSeconds = (seconds >= 0.0) ? seconds : 0.0;
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

  const auto delta = std::chrono::duration<double>(seconds - m_steadyBaseRowSeconds);
  return m_steadyBase + std::chrono::duration_cast<std::chrono::steady_clock::duration>(delta);
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
  // replayChannelSpans() marshals blocking and pumps this thread's event loop, so a queued
  // updateData() or close click can re-enter here and tear state down under the byte views the
  // outer call already handed to the builder.
  // code-verify on
  if (m_injecting)
    return;

  const QScopedValueRollback<bool> reentry_guard(m_injecting, true);

  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile) {
    injectFrame(quickPlotPayload(row));
    return;
  }

  const qsizetype count = splitDataCells(row);
  if (count <= 0) [[unlikely]]
    return;

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  const auto timestamp      = rowSteadyTimestamp(row);

  if (!m_multiSource) {
    frameBuilder.replayChannelSpans(0, m_dataSpans.constData(), count, timestamp);
    return;
  }

  for (auto it = m_sourceColumnsByIndex.constBegin(); it != m_sourceColumnsByIndex.constEnd();
       ++it) {
    const auto& orderedCols = it.value();
    QVarLengthArray<QByteArrayView, 64> cells;
    cells.reserve(orderedCols.size());
    bool present = false;
    for (int col : orderedCols) {
      const QByteArrayView cell =
        (col >= 0 && col < count) ? m_dataSpans.at(col) : QByteArrayView();
      present = present || !cell.isEmpty();
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

  if (!m_multiSource) {
    static auto& connectionManager = IO::ConnectionManager::instance();
    connectionManager.processPayload(frame);
    return;
  }

  const QString row = QString::fromUtf8(frame).trimmed();
  const auto fields = DataModel::splitReplayRow(row);

  QMap<int, QStringList> sourceFields;
  QSet<int> sourcesPresent;
  for (auto it = m_sourceColumnsByIndex.constBegin(); it != m_sourceColumnsByIndex.constEnd();
       ++it) {
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

  static auto& connectionManager = IO::ConnectionManager::instance();
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

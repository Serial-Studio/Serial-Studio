/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include "Export.h"

#include <QDateTime>
#include <QDir>

#include "AppState.h"
#include "CSV/Player.h"
#include "DataModel/FrameBuilder.h"
#include "IO/ConnectionManager.h"
#include "MDF4/Player.h"
#include "Misc/WorkspaceManager.h"
#include "SerialStudio.h"

/**
 * @brief Escapes a CSV field per RFC 4180 and neutralizes leading formula-injection chars.
 *        Numeric fields are exempt: a plain number is inert in every spreadsheet, and
 *        prefixing negatives ("-0.5" -> "'-0.5") silently corrupts recordings on replay.
 */
static QString escapeCsvField(const QString& s)
{
  QString out = s;
  if (!out.isEmpty()) {
    const QChar c     = out.at(0);
    const bool danger = c == QChar('=') || c == QChar('+') || c == QChar('-') || c == QChar('@')
                     || c == QChar('\t') || c == QChar('\r');
    if (danger) {
      bool numeric = false;
      (void)SerialStudio::toDouble(out, &numeric);
      if (!numeric)
        out.prepend(QChar('\''));
    }
  }

  const bool needs = out.contains(QChar(',')) || out.contains(QChar('"'))
                  || out.contains(QChar('\n')) || out.contains(QChar('\r'))
                  || out.contains(QChar('\t'));
  if (!needs)
    return out;

  out.replace(QChar('"'), QStringLiteral("\"\""));
  return QStringLiteral("\"%1\"").arg(out);
}

//--------------------------------------------------------------------------------------------------
// ExportWorker implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the CSV worker with its snapshot timer. The timer is a child so it migrates
 *        with the worker to its thread; it stays stopped until a non-zero interval arrives.
 */
CSV::ExportWorker::ExportWorker(
  moodycamel::ReaderWriterQueue<DataModel::TimestampedFramePtr>* queue,
  std::atomic<bool>* enabled,
  std::atomic<size_t>* queueSize)
  : DataModel::FrameConsumerWorker<DataModel::TimestampedFramePtr>(queue, enabled, queueSize)
  , m_snapshotIntervalMs(0)
  , m_snapshotTimer(new QTimer(this))
{
  m_snapshotTimer->setTimerType(Qt::PreciseTimer);
  connect(m_snapshotTimer, &QTimer::timeout, this, &ExportWorker::writeSnapshotRow);
}

/**
 * @brief Returns whether the CSV file is currently open.
 */
bool CSV::ExportWorker::isResourceOpen() const
{
  return m_csvFile.isOpen();
}

/**
 * @brief Applies the snapshot interval on the worker thread: 0 restores per-frame rows and stops
 *        the timer; a positive value switches row writing to the fixed cadence (spec 0023).
 */
void CSV::ExportWorker::setSnapshotIntervalMs(int interval)
{
  m_snapshotIntervalMs = qMax(0, interval);

  if (m_snapshotIntervalMs > 0) {
    m_snapshotTimer->setInterval(m_snapshotIntervalMs);
    m_snapshotTimer->start();
  }

  else
    m_snapshotTimer->stop();
}

/**
 * @brief Writes one interval-mode snapshot row from the forward-fill map. Drains the pending queue
 *        first so cell staleness is bounded by the snapshot interval rather than the 1000 ms batch
 *        timer at low frame rates, and writes nothing before the session's first frame: the file
 *        only exists once processItems() has seen data (spec 0023).
 */
void CSV::ExportWorker::writeSnapshotRow()
{
  if (!consumerEnabled() || m_snapshotIntervalMs <= 0)
    return;

  processData();

  if (!m_csvFile.isOpen() || m_schema.columns.empty())
    return;

  writeRow(DataModel::TimestampedFrame::SteadyClock::now());
  m_textStream.flush();
}

/**
 * @brief Closes the currently open CSV file and resets worker state.
 */
void CSV::ExportWorker::closeResources()
{
  if (!m_csvFile.isOpen())
    return;

  m_csvFile.close();
  m_schema = DataModel::ExportSchema{};
  m_textStream.setDevice(nullptr);
  m_lastFinalValues.clear();
  DataModel::clear_frame(m_templateFrame);
}

/**
 * @brief Stores the schema template frame; must run on the worker thread (queued invoke) so
 *        the assignment never races processItems() or closeResources().
 */
void CSV::ExportWorker::setTemplateFrame(const DataModel::Frame& frame)
{
  m_templateFrame = frame;
}

/**
 * @brief Processes a batch of CSV frames, creating the file if needed. Per-frame mode (interval
 *        0) writes one row per frame; interval mode only refreshes the forward-fill map and
 *        leaves row writing to the snapshot timer.
 */
void CSV::ExportWorker::processItems(const std::vector<DataModel::TimestampedFramePtr>& items)
{
  if (items.empty())
    return;

  if (!m_csvFile.isOpen()) {
    if (!m_templateFrame.groups.empty())
      createCsvFile(m_templateFrame);
    else
      createCsvFile((*items.begin())->data);

    if (m_schema.columns.empty())
      return;

    m_referenceTimestamp = (*items.begin())->timestamp;
    resetMonotonicClock();
  }

  for (const auto& i : items) {
    for (const auto& g : i->data.groups)
      for (const auto& d : g.datasets)
        m_lastFinalValues[d.uniqueId] = d.value.simplified();

    if (m_snapshotIntervalMs == 0)
      writeRow(i->timestamp);
  }

  if (m_snapshotIntervalMs == 0)
    m_textStream.flush();
}

/**
 * @brief Writes one timestamped row across the full schema from the forward-fill map; shared by
 *        the per-frame path (frame timestamps) and the snapshot path (now() at the tick).
 */
void CSV::ExportWorker::writeRow(const DataModel::TimestampedFrame::SteadyTimePoint& timestamp)
{
  const qint64 nanoseconds = monotonicFrameNs(timestamp, m_referenceTimestamp);
  const double seconds     = static_cast<double>(nanoseconds) / 1'000'000'000.0;
  m_textStream << QString::number(seconds, 'f', 9);

  const int colCount = static_cast<int>(m_schema.columns.size());
  for (int j = 0; j < colCount; ++j) {
    const int uid = m_schema.columns[static_cast<size_t>(j)].uniqueId;
    m_textStream << ',' << escapeCsvField(m_lastFinalValues.value(uid, QString()));
  }

  m_textStream << '\n';
}

//--------------------------------------------------------------------------------------------------
// Stream-block CSV sink (spec 0051 M5)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the stream-block CSV worker.
 */
CSV::StreamExportWorker::StreamExportWorker(
  moodycamel::ReaderWriterQueue<IO::StreamBlockItemPtr>* queue,
  std::atomic<bool>* enabled,
  std::atomic<size_t>* queueSize)
  : FrameConsumerWorker(queue, enabled, queueSize)
{}

/**
 * @brief Returns true while any per-source stream CSV file is open.
 */
bool CSV::StreamExportWorker::isResourceOpen() const
{
  return !m_files.empty();
}

/**
 * @brief Closes every per-source stream CSV file.
 */
void CSV::StreamExportWorker::closeResources()
{
  for (auto& [sourceId, state] : m_files) {
    if (state.stream)
      state.stream->flush();

    if (state.file)
      state.file->close();
  }

  m_files.clear();
}

/**
 * @brief Returns (creating on first sight) the file state for @p block's source; the header
 *        row names each dataset column by uniqueId.
 */
CSV::StreamExportWorker::FileState* CSV::StreamExportWorker::fileFor(
  const IO::StreamBlockItem& block)
{
  auto it = m_files.find(block.sourceId);
  if (it != m_files.end())
    return &it->second;

  const auto dt       = QDateTime::currentDateTime();
  const auto fileName = dt.toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss"))
                      + QStringLiteral("_stream_source%1.csv").arg(block.sourceId);

  static auto& workspaceManager = Misc::WorkspaceManager::instance();
  QDir dir(workspaceManager.path(QStringLiteral("CSV")));

  FileState state;
  state.file = std::make_unique<QFile>(dir.filePath(fileName));
  if (!state.file->open(QIODevice::WriteOnly | QIODevice::Text)) {
    qWarning() << "[CSV] cannot open stream export file" << state.file->fileName();
    return nullptr;
  }

  state.stream = std::make_unique<QTextStream>(state.file.get());
  state.start  = block.t0;

  *state.stream << "Time (s)";
  for (const int uniqueId : block.uniqueIds)
    *state.stream << ",ds" << uniqueId;

  *state.stream << '\n';

  auto [inserted, ok] = m_files.emplace(block.sourceId, std::move(state));
  return ok ? &inserted->second : nullptr;
}

/**
 * @brief Writes one full-rate block: every sample row carries t0 + i * dt relative to the
 *        file's first block (source-owns-time; never the monotonic bump).
 */
void CSV::StreamExportWorker::writeBlock(FileState& state, const IO::StreamBlockItem& block)
{
  const double baseSec = std::chrono::duration<double>(block.t0 - state.start).count();
  const double dtSec   = std::chrono::duration<double>(block.dt).count();
  auto& stream         = *state.stream;

  for (qsizetype i = 0; i < block.frames; ++i) {
    stream << QString::number(baseSec + static_cast<double>(i) * dtSec, 'f', 9);
    for (const auto& channel : block.channels) {
      stream << ',';
      if (static_cast<std::size_t>(i) < channel.size())
        stream << QString::number(channel[static_cast<std::size_t>(i)], 'g', 10);
    }

    stream << '\n';
  }
}

/**
 * @brief Writes a batch of stream blocks to their per-source files.
 */
void CSV::StreamExportWorker::processItems(const std::vector<IO::StreamBlockItemPtr>& items)
{
  for (const auto& block : items) {
    if (!block || block->frames <= 0)
      continue;

    auto* state = fileFor(*block);
    if (!state)
      continue;

    writeBlock(*state, *block);
  }

  for (auto& [sourceId, state] : m_files)
    if (state.stream)
      state.stream->flush();
}

/**
 * @brief Constructs the stream-block CSV facade (worker created eagerly like the frame sink).
 */
CSV::StreamExport::StreamExport()
  : DataModel::FrameConsumer<IO::StreamBlockItemPtr>(
      {.queueCapacity = 4096, .flushThreshold = 256, .timerIntervalMs = 500})
{
  initializeWorker();
  setConsumerEnabled(false);
}

/**
 * @brief GUI-thread ingest slot: the single producer for the worker's SPSC queue, fed by every
 *        stream worker's queued blockReady signal (block rate, never per sample).
 */
void CSV::StreamExport::ingestBlock(const IO::StreamBlockItemPtr& block)
{
  if (block)
    enqueueData(block);
}

/**
 * @brief Flushes and closes every per-source stream file (queued to the worker).
 */
void CSV::StreamExport::closeFiles()
{
  if (m_worker)
    QMetaObject::invokeMethod(m_worker, "close", Qt::QueuedConnection);
}

/**
 * @brief Factory method creating the stream-block worker.
 */
DataModel::FrameConsumerWorkerBase* CSV::StreamExport::createWorker()
{
  return new StreamExportWorker(&m_pendingQueue, &m_consumerEnabled, &m_queueSize);
}

/**
 * @brief Creates a new CSV file and writes the header row from the frame schema.
 */
void CSV::ExportWorker::createCsvFile(const DataModel::Frame& frame)
{
  const auto dt       = QDateTime::currentDateTime();
  const auto fileName = dt.toString("yyyy-MM-dd_HH-mm-ss") + ".csv";

  static auto& workspaceManager = Misc::WorkspaceManager::instance();
  const auto subdir             = workspaceManager.path("CSV");
  QString safeTitle             = frame.title;
  safeTitle.remove(QChar('/'));
  safeTitle.remove(QChar('\\'));
  safeTitle.remove(QChar(':'));
  safeTitle.remove(QChar('*'));
  safeTitle.remove(QChar('?'));
  safeTitle.remove(QChar('"'));
  safeTitle.remove(QChar('<'));
  safeTitle.remove(QChar('>'));
  safeTitle.remove(QChar('|'));
  safeTitle.remove(QChar('\0'));
  safeTitle.remove(QStringLiteral(".."));
  safeTitle = safeTitle.simplified();

  int keepCsv = 0;
  for (int i = safeTitle.size(); i > 0; --i) {
    const QChar c = safeTitle.at(i - 1);
    if (c != QChar('.') && c != QChar(' ')) {
      keepCsv = i;
      break;
    }
  }
  safeTitle.truncate(keepCsv);

  if (safeTitle.isEmpty())
    safeTitle = QStringLiteral("Untitled");

  const QString path = QString("%1/%2/").arg(subdir, safeTitle);

  QDir dir(path);
  if (!dir.exists() && !dir.mkpath(".")) {
    qWarning() << "Failed to create directory:" << path;
    return;
  }

  m_csvFile.setFileName(dir.filePath(fileName));
  if (!m_csvFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qWarning() << "Cannot open CSV file for writing:" << dir.filePath(fileName);
    return;
  }

  m_lastFinalValues.clear();

  m_textStream.setDevice(&m_csvFile);
  m_textStream.setGenerateByteOrderMark(true);
  m_textStream.setEncoding(QStringConverter::Utf8);

  m_schema = DataModel::buildExportSchema(frame);

  m_textStream << "Elapsed (s)";
  for (const auto& col : m_schema.columns) {
    auto label = QString("%1/%2").arg(col.groupTitle, col.title).simplified();
    if (!col.sourceTitle.isEmpty())
      label = col.sourceTitle + "/" + label;

    m_textStream << ',' << escapeCsvField(label);
  }

  m_textStream << '\n';

  Q_EMIT resourceOpenChanged();
}

//--------------------------------------------------------------------------------------------------
// Export constructor, destructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the CSV export manager and initializes the worker.
 */
CSV::Export::Export()
  : DataModel::FrameConsumer<DataModel::TimestampedFramePtr>(
      {.queueCapacity = 8192, .flushThreshold = 1024, .timerIntervalMs = 1000})
  , m_isOpen(false)
  , m_persistSettings(true)
  , m_exportInterval(0)
{
  initializeWorker();
  connect(m_worker,
          &ExportWorker::resourceOpenChanged,
          this,
          &Export::onWorkerOpenChanged,
          Qt::QueuedConnection);

  setExportEnabled(m_settings.value("CSVExport", false).toBool());
  setExportInterval(m_settings.value("CSVExportInterval", 0).toInt());
}

/**
 * @brief Default destructor.
 */
CSV::Export::~Export() = default;

/**
 * @brief Returns the singleton CSV export instance.
 */
CSV::Export& CSV::Export::instance()
{
  static Export singleton;
  return singleton;
}

/**
 * @brief Factory method that creates the CSV export worker.
 */
DataModel::FrameConsumerWorkerBase* CSV::Export::createWorker()
{
  return new ExportWorker(&m_pendingQueue, &m_consumerEnabled, &m_queueSize);
}

//--------------------------------------------------------------------------------------------------
// State access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether a CSV file is currently open for writing.
 */
bool CSV::Export::isOpen() const
{
  return m_isOpen.load(std::memory_order_relaxed);
}

/**
 * @brief Returns whether CSV export is enabled.
 */
bool CSV::Export::exportEnabled() const
{
  return consumerEnabled();
}

/**
 * @brief Returns the snapshot interval in milliseconds (0 = one row per frame).
 */
int CSV::Export::exportInterval() const
{
  return m_exportInterval;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Flushes queued data and closes the currently open CSV file.
 */
void CSV::Export::closeFile()
{
  auto* worker = static_cast<ExportWorker*>(m_worker);
  QMetaObject::invokeMethod(worker, "close", Qt::QueuedConnection);
  m_streamExport.closeFiles();
}

/**
 * @brief Syncs cached open state from the worker and emits openChanged.
 */
void CSV::Export::onWorkerOpenChanged()
{
  auto* worker = static_cast<ExportWorker*>(m_worker);
  m_isOpen.store(worker->isResourceOpen(), std::memory_order_relaxed);
  Q_EMIT openChanged();
}

/**
 * @brief Wires IO and app-state signals that control export behaviour.
 */
void CSV::Export::setupExternalConnections()
{
  connect(
    &IO::ConnectionManager::instance(), &IO::ConnectionManager::connectedChanged, this, [this] {
      if (IO::ConnectionManager::instance().isConnected()) {
        auto* worker = static_cast<ExportWorker*>(m_worker);
        DataModel::Frame frame;
        if (AppState::instance().operationMode() == SerialStudio::ProjectFile) {
          static auto& builder = DataModel::FrameBuilder::instance();
          builder.invokeOnBuilderThreadBlocking(
            [&frame] { frame = DataModel::FrameBuilder::instance().frame(); });
        }

        QMetaObject::invokeMethod(
          worker,
          [worker, frame = std::move(frame)] { worker->setTemplateFrame(frame); },
          Qt::QueuedConnection);
      }

      else {
        closeFile();
      }
    });
  connect(&IO::ConnectionManager::instance(), &IO::ConnectionManager::pausedChanged, this, [this] {
    if (IO::ConnectionManager::instance().paused())
      closeFile();
  });

  connect(&AppState::instance(), &AppState::operationModeChanged, this, [this] {
    if (AppState::instance().operationMode() == SerialStudio::ConsoleOnly && exportEnabled())
      setExportEnabled(false);
  });
}

/**
 * @brief Toggles whether export-enabled changes get written to QSettings.
 */
void CSV::Export::setSettingsPersistent(const bool persistent)
{
  m_persistSettings = persistent;
}

/**
 * @brief Sets the snapshot interval in milliseconds (0 = one row per frame) and forwards it to
 *        the worker thread; applies live to an open recording (spec 0023).
 */
void CSV::Export::setExportInterval(const int interval)
{
  const int clamped = qMax(0, interval);
  m_exportInterval  = clamped;

  auto* worker = static_cast<ExportWorker*>(m_worker);
  QMetaObject::invokeMethod(
    worker, [worker, clamped] { worker->setSnapshotIntervalMs(clamped); }, Qt::QueuedConnection);

  if (m_persistSettings)
    m_settings.setValue("CSVExportInterval", clamped);

  Q_EMIT intervalChanged();
}

/**
 * @brief Enables or disables CSV export, closing the file on disable.
 */
void CSV::Export::setExportEnabled(const bool enabled)
{
  static auto& appState = AppState::instance();
  const bool allow      = enabled && appState.operationMode() != SerialStudio::ConsoleOnly;

  if (!allow && isOpen())
    closeFile();

  setConsumerEnabled(allow);
  m_streamExport.setConsumerEnabled(allow);
  if (!allow)
    m_streamExport.closeFiles();

  if (m_persistSettings)
    m_settings.setValue("CSVExport", allow);

  Q_EMIT enabledChanged();
}

/**
 * @brief Returns the typed stream-block sink (spec 0051 M5); enable state follows exportEnabled.
 */
CSV::StreamExport& CSV::Export::streamSink() noexcept
{
  return m_streamExport;
}

//--------------------------------------------------------------------------------------------------
// Hotpath data processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enqueues a frame for asynchronous CSV export.
 */
void CSV::Export::hotpathTxFrame(const DataModel::TimestampedFramePtr& frame)
{
  if (!exportEnabled() || SerialStudio::isAnyPlayerOpen())
    return;

  enqueueData(frame);
}

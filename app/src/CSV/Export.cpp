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

#include <charconv>
#include <cstdio>

#if defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) \
  && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 130300
#  define SS_APPLE_NO_FLOAT_TO_CHARS 1
#  include <xlocale.h>
#endif

#include <algorithm>
#include <limits>
#include <QDateTime>
#include <QDir>
#include <QVarLengthArray>

#include "AppState.h"
#include "CSV/Player.h"
#include "DataModel/FrameBuilder.h"
#include "IO/ConnectionManager.h"
#include "MDF4/Player.h"
#include "Misc/WorkspaceManager.h"
#include "SerialStudio.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Reorder window keeping rows time-ordered across sources (spec 0055 D3/D7); T34 pins the sweep
static constexpr qint64 kReorderWindowNs = 250'000'000LL;

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

/**
 * @brief Appends @p value to @p dst byte-identically to QString::number(value, fmt, precision),
 *        without the per-cell QString allocation: both are C-locale %f / %g. Apple ships float
 *        std::to_chars only from macOS 13.3, so older targets use snprintf_l with the NULL (C)
 *        locale; the buffer covers the fixed worst case (sign + 309 digits + '.' + precision).
 */
static void appendDouble(QByteArray& dst, double value, std::chars_format fmt, int precision)
{
  SS_ASSERT(precision >= 0, return);
  SS_ASSERT(fmt == std::chars_format::fixed || fmt == std::chars_format::general, return);

  char buf[352];
#ifdef SS_APPLE_NO_FLOAT_TO_CHARS
  const char* spec = (fmt == std::chars_format::fixed) ? "%.*f" : "%.*g";
  const int len    = snprintf_l(buf, sizeof(buf), nullptr, spec, precision, value);
  SS_ASSERT(len > 0 && static_cast<size_t>(len) < sizeof(buf), return);
  dst.append(buf, static_cast<qsizetype>(len));
#else
  const auto res = std::to_chars(buf, buf + sizeof(buf), value, fmt, precision);
  SS_ASSERT(res.ec == std::errc(), return);
  dst.append(buf, static_cast<qsizetype>(res.ptr - buf));
#endif
}

/**
 * @brief Byte-level double formatter the sparse merger shares with the header writer.
 */
void CSV::appendCsvDouble(QByteArray& dst, double value, bool fixed, int precision)
{
  appendDouble(
    dst, value, fixed ? std::chars_format::fixed : std::chars_format::general, precision);
}

/**
 * @brief RFC-4180 field escape the sparse merger shares with the header writer.
 */
QByteArray CSV::escapeCsvBytes(const QString& field)
{
  return escapeCsvField(field.simplified()).toUtf8();
}

//--------------------------------------------------------------------------------------------------
// ExportWorker implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the CSV worker with its snapshot timer. The timer is a child so it migrates
 *        with the worker to its thread; it stays stopped until a non-zero interval arrives.
 */
CSV::ExportWorker::ExportWorker(moodycamel::ReaderWriterQueue<DataModel::DataBlockPtr>* queue,
                                std::atomic<bool>* enabled,
                                std::atomic<size_t>* queueSize)
  : DataModel::FrameConsumerWorker<DataModel::DataBlockPtr>(queue, enabled, queueSize)
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
 * @brief Applies the snapshot interval on the worker thread: 0 restores sparse full-rate rows and
 *        stops the timer; a positive value switches to dense forward-filled rows on a fixed
 *        cadence (spec 0023, kept by spec-0055 D4 at its disabled default).
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
 * @brief Writes one interval-mode row from the forward-fill map. Drains the pending queue first so
 *        cell staleness is bounded by the interval rather than the batch timer, and writes nothing
 *        before the session's first block: the file only exists once data has arrived.
 */
void CSV::ExportWorker::writeSnapshotRow()
{
  if (!consumerEnabled() || m_snapshotIntervalMs <= 0)
    return;

  processData();

  if (!m_csvFile.isOpen() || m_schema.columns.empty())
    return;

  writeSnapshotRowNow(DataModel::TimestampedFrame::SteadyClock::now());
}

/**
 * @brief Closes the currently open CSV file and resets worker state, flushing whatever the reorder
 *        window still holds so the tail of a recording is never lost.
 */
void CSV::ExportWorker::closeResources()
{
  if (!m_csvFile.isOpen())
    return;

  flushReadyRows(std::numeric_limits<qint64>::max());

  m_csvFile.close();
  m_schema = DataModel::ExportSchema{};
  m_merger.clear();
  m_rowBuffer.clear();
  m_lastFinalValues.clear();
  m_structure.clear();
}

/**
 * @brief Stores the schema template frame; must run on the worker thread (queued invoke) so the
 *        assignment never races processItems() or closeResources().
 */
void CSV::ExportWorker::setTemplateFrame(const DataModel::Frame& frame)
{
  m_structure.setTemplateFrame(frame);
}

/**
 * @brief Adopts the structure the pipeline publishes when the connect-time fetch came back empty
 *        (QuickPlot derives its datasets from the first frame, so at connect there is none).
 */
void CSV::ExportWorker::applyPublishedStructure(const DataModel::Frame& frame)
{
  m_structure.applyPublishedStructure(frame, isResourceOpen());
}

/**
 * @brief Ingests a batch of blocks, creating the file on first data. Sparse mode buffers them for
 *        the reorder window and flushes everything older than it; interval mode only refreshes the
 *        forward-fill map and leaves row writing to the snapshot timer.
 */
void CSV::ExportWorker::processItems(const std::vector<DataModel::DataBlockPtr>& items)
{
  if (items.empty())
    return;

  if (!m_csvFile.isOpen()) {
    if (m_structure.hasStructure())
      createCsvFile(m_structure.templateFrame());

    if (m_schema.columns.empty())
      return;

    m_referenceTimestamp = items.front()->t0;
    for (const auto& block : items)
      if (block && block->samples > 0 && block->t0 < m_referenceTimestamp)
        m_referenceTimestamp = block->t0;

    resetMonotonicClock();
  }

  for (const auto& block : items) {
    if (!block || block->samples <= 0)
      continue;

    if (m_snapshotIntervalMs > 0) {
      const auto last = static_cast<std::size_t>(block->samples - 1);
      for (const auto& column : block->columns)
        m_lastFinalValues[column.uniqueId] = column.hasText
                                             ? column.text[last].simplified()
                                             : QString::number(column.values[last], 'g', 10);

      continue;
    }

    bufferBlock(block);
  }

  if (m_snapshotIntervalMs > 0)
    return;

  if (!m_merger.empty())
    flushReadyRows(m_merger.newestNs() - kReorderWindowNs);

  (void)m_csvFile.flush();
}

/**
 * @brief Buffers one block for the merge, resolving its schema columns and per-sample times once.
 *        Every sample keeps the instant its own source stamped; an irregular block only takes the
 *        per-source tie-break, so two frames landing on the same coarse-clock nanosecond stay
 *        distinct rows without a second source's samples being rewritten behind the first (B1).
 */
void CSV::ExportWorker::bufferBlock(const DataModel::DataBlockPtr& block)
{
  const bool uniform = DataModel::uniform_grid(*block);

  std::vector<qint64> times;
  times.reserve(static_cast<std::size_t>(block->samples));
  for (qsizetype i = 0; i < block->samples; ++i) {
    const auto stamp = DataModel::sample_time(*block, i);
    qint64 offset =
      std::chrono::duration_cast<std::chrono::nanoseconds>(stamp - m_referenceTimestamp).count();
    if (!uniform)
      offset = monotonicSourceNs(block->sourceId, offset);

    times.push_back(std::max<qint64>(0, offset));
  }

  m_merger.addBlock(block, std::move(times));
}

/**
 * @brief Emits every buffered instant at or before @p cutoffNs into the open file.
 */
void CSV::ExportWorker::flushReadyRows(qint64 cutoffNs)
{
  bool failed = false;
  m_merger.flush(cutoffNs, [this, &failed](const QByteArray& row) {
    if (failed)
      return;

    failed = m_csvFile.write(row) < row.size();
  });

  if (!failed)
    return;

  qWarning() << "[CSV] row write failed, closing export:" << m_csvFile.fileName();
  closeResources();
  Q_EMIT resourceOpenChanged();
}

/**
 * @brief Writes one dense forward-filled row across the full schema (interval mode). A failed write
 *        closes the export exactly as the sparse path does: leaving the file "open" dropped every
 *        later row in silence (B14).
 */
void CSV::ExportWorker::writeSnapshotRowNow(
  const DataModel::TimestampedFrame::SteadyTimePoint& timestamp)
{
  const qint64 nanoseconds = std::max<qint64>(0, monotonicFrameNs(timestamp, m_referenceTimestamp));

  m_rowBuffer.resize(0);
  appendDouble(
    m_rowBuffer, static_cast<double>(nanoseconds) / 1'000'000'000.0, std::chars_format::fixed, 9);

  for (const auto& column : m_schema.columns) {
    m_rowBuffer += ',';
    m_rowBuffer += escapeCsvField(m_lastFinalValues.value(column.uniqueId, QString())).toUtf8();
  }

  m_rowBuffer          += '\n';
  const qint64 written  = m_csvFile.write(m_rowBuffer);
  if (written < m_rowBuffer.size()) [[unlikely]] {
    qWarning() << "[CSV] snapshot row write failed, closing export:" << m_csvFile.fileName();
    closeResources();
    Q_EMIT resourceOpenChanged();
    return;
  }

  (void)m_csvFile.flush();
}

/**
 * @brief Creates a new CSV file and writes the header row from the frame schema.
 */
void CSV::ExportWorker::createCsvFile(const DataModel::Frame& frame)
{
  const auto dt       = QDateTime::currentDateTime();
  const auto fileName = dt.toString("yyyy-MM-dd_HH-mm-ss") + ".csv";

  const QDir dir = DataModel::ExportStructure::sessionDir(
    QStringLiteral("CSV"), frame.title, QStringLiteral("Untitled"));
  if (!dir.exists())
    return;

  m_csvFile.setFileName(dir.filePath(fileName));
  if (!m_csvFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qWarning() << "Cannot open CSV file for writing:" << dir.filePath(fileName);
    return;
  }

  m_lastFinalValues.clear();
  m_schema = DataModel::buildExportSchema(frame);
  m_merger.setSchema(m_schema);

  QByteArray header("\xEF\xBB\xBF", 3);
  header += "Elapsed (s)";
  for (const auto& col : m_schema.columns) {
    auto label = QString("%1/%2").arg(col.groupTitle, col.title).simplified();
    if (!col.sourceTitle.isEmpty())
      label = col.sourceTitle + "/" + label;

    header += ',';
    header += escapeCsvField(label).toUtf8();
  }

  header += '\n';
  if (m_csvFile.write(header) < header.size()) {
    qWarning() << "[CSV] cannot write export header" << m_csvFile.fileName();
    m_csvFile.close();
    return;
  }

  Q_EMIT resourceOpenChanged();
}

//--------------------------------------------------------------------------------------------------
// Export constructor, destructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the CSV export manager and initializes the worker.
 */
CSV::Export::Export()
  : DataModel::FrameConsumer<DataModel::DataBlockPtr>(
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
    &DataModel::FrameBuilder::instance(),
    &DataModel::FrameBuilder::structurePublished,
    this,
    [this](int, const DataModel::Frame& frame) {
      auto* worker = static_cast<ExportWorker*>(m_worker);
      SS_ASSERT(worker != nullptr, return);

      QMetaObject::invokeMethod(
        worker, [worker, frame] { worker->applyPublishedStructure(frame); }, Qt::QueuedConnection);
    });
  connect(&DataModel::FrameBuilder::instance(),
          &DataModel::FrameBuilder::sessionStructureReady,
          this,
          [this](const DataModel::Frame& frame) {
            auto* worker = static_cast<ExportWorker*>(m_worker);
            SS_ASSERT(worker != nullptr, return);

            QMetaObject::invokeMethod(
              worker, [worker, frame] { worker->setTemplateFrame(frame); }, Qt::QueuedConnection);
          });
  // code-verify off
  // Closing on the builder's session boundary rather than on connectedChanged/pausedChanged is
  // what keeps the last display tick (A2): the builder flushes its open blocks into this sink's
  // queue before emitting, and close() drains that queue before closing the file.
  // code-verify on
  connect(&DataModel::FrameBuilder::instance(),
          &DataModel::FrameBuilder::sessionBoundary,
          this,
          [this](bool connected, bool paused) {
            if (!connected || paused)
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

  if (m_persistSettings)
    m_settings.setValue("CSVExport", allow);

  Q_EMIT enabledChanged();
}

//--------------------------------------------------------------------------------------------------
// Hotpath data processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enqueues one block for asynchronous CSV export. The single producer for this SPSC queue
 *        is the pipeline thread, for both lanes (spec 0055 D8).
 */
void CSV::Export::ingestBlock(const DataModel::DataBlockPtr& block)
{
  if (!block || !exportEnabled() || SerialStudio::isAnyPlayerOpen())
    return;

  enqueueData(block);
}

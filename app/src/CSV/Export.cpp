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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "Export.h"

#include "AppState.h"
#include "CSV/Player.h"
#include "DataModel/FrameBuilder.h"
#include "IO/ConnectionManager.h"
#include "MDF4/Player.h"
#include "Misc/WorkspaceManager.h"

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Client.h"
#endif

#include <QDateTime>
#include <QDir>

namespace {

// RFC 4180 CSV field escape: quote fields containing separators, quotes, or
// newlines and double any embedded quotes. Returning the input unchanged for
// safe values keeps the common-case hotpath allocation-free.
static QString escapeCsvField(const QString& s)
{
  // Bail early when no special characters are present
  const bool needs = s.contains(QChar(',')) || s.contains(QChar('"')) || s.contains(QChar('\n'))
                  || s.contains(QChar('\r')) || s.contains(QChar('\t'));
  if (!needs)
    return s;

  // Double-up any embedded quotes per RFC 4180 and wrap the result
  QString out = s;
  out.replace(QChar('"'), QStringLiteral("\"\""));
  return QStringLiteral("\"%1\"").arg(out);
}

}  // namespace

//--------------------------------------------------------------------------------------------------
// ExportWorker implementation
//--------------------------------------------------------------------------------------------------

bool CSV::ExportWorker::isResourceOpen() const
{
  return m_csvFile.isOpen();
}

/**
 * @brief Closes the currently open CSV file and resets worker state.
 */
void CSV::ExportWorker::closeResources()
{
  // Nothing to do if the file isn't open
  if (!m_csvFile.isOpen())
    return;

  m_csvFile.close();
  m_schema = DataModel::ExportSchema{};
  m_textStream.setDevice(nullptr);
  DataModel::clear_frame(m_templateFrame);
}

/**
 * @brief Processes a batch of CSV frames, creating the file if needed.
 */
void CSV::ExportWorker::processItems(const std::vector<DataModel::TimestampedFramePtr>& items)
{
  // Validate batch is non-empty
  if (items.empty())
    return;

  // Open a new CSV file if needed
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

  const int colCount = static_cast<int>(m_schema.columns.size());

  for (const auto& i : items) {
    const qint64 nanoseconds = monotonicFrameNs(i->timestamp, m_referenceTimestamp);
    const double seconds     = static_cast<double>(nanoseconds) / 1'000'000'000.0;
    m_textStream << QString::number(seconds, 'f', 9);

    // Collect post-transform values by uniqueId — CSV exports final values
    // only, matching what users see on the dashboard. Raw pre-transform
    // values remain accessible via the SQLite session DB and MDF4 raw
    // channels when byte-level fidelity is needed.
    QMap<int, QString> finalValues;
    for (const auto& g : i->data.groups)
      for (const auto& d : g.datasets)
        finalValues[d.uniqueId] = d.value.simplified();

    // Write one column per dataset in schema order, escaping per RFC 4180
    // so transform outputs containing commas/quotes/newlines don't corrupt
    // the row layout for downstream consumers.
    for (int j = 0; j < colCount; ++j) {
      const int uid = m_schema.columns[static_cast<size_t>(j)].uniqueId;
      m_textStream << ',' << escapeCsvField(finalValues.value(uid, QString()));
    }

    m_textStream << '\n';
  }

  m_textStream.flush();
}

/**
 * @brief Creates a new CSV file and writes the header row from the frame schema.
 */
void CSV::ExportWorker::createCsvFile(const DataModel::Frame& frame)
{
  // Build the output file path from the current timestamp
  const auto dt       = QDateTime::currentDateTime();
  const auto fileName = dt.toString("yyyy-MM-dd_HH-mm-ss") + ".csv";

  // Sanitize frame title to prevent path traversal
  const auto subdir = Misc::WorkspaceManager::instance().path("CSV");
  QString safeTitle = frame.title;
  safeTitle.remove(QChar('/'));
  safeTitle.remove(QChar('\\'));
  safeTitle.remove(QStringLiteral(".."));
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

  m_textStream.setDevice(&m_csvFile);
  m_textStream.setGenerateByteOrderMark(true);
  m_textStream.setEncoding(QStringConverter::Utf8);

  // Build schema from frame
  m_schema = DataModel::buildExportSchema(frame);

  // Write header. The first column is elapsed seconds from the first sample
  // in this session — NOT a calendar timestamp — so the label must match.
  // Escape titles in case group/dataset names contain commas or quotes.
  m_textStream << "Elapsed (s)";
  for (const auto& col : m_schema.columns) {
    const auto label = QString("%1/%2").arg(col.groupTitle, col.title).simplified();
    m_textStream << ',' << escapeCsvField(label);
  }

  m_textStream << '\n';

  Q_EMIT resourceOpenChanged();
}

//--------------------------------------------------------------------------------------------------
// Export constructor, destructor & singleton access functions
//--------------------------------------------------------------------------------------------------

CSV::Export::Export()
  : DataModel::FrameConsumer<DataModel::TimestampedFramePtr>(
      {.queueCapacity = 8192, .flushThreshold = 1024, .timerIntervalMs = 1000})
  , m_isOpen(false)
{
  // Set up background worker and connect state change signal
  initializeWorker();
  connect(m_worker,
          &ExportWorker::resourceOpenChanged,
          this,
          &Export::onWorkerOpenChanged,
          Qt::QueuedConnection);
}

CSV::Export::~Export() = default;

CSV::Export& CSV::Export::instance()
{
  static Export singleton;
  return singleton;
}

DataModel::FrameConsumerWorkerBase* CSV::Export::createWorker()
{
  return new ExportWorker(&m_pendingQueue, &m_consumerEnabled, &m_queueSize);
}

//--------------------------------------------------------------------------------------------------
// State access functions
//--------------------------------------------------------------------------------------------------

bool CSV::Export::isOpen() const
{
  return m_isOpen.load(std::memory_order_relaxed);
}

bool CSV::Export::exportEnabled() const
{
  return consumerEnabled();
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
  // Sync cached open state from worker thread
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
        // Only cache the template in ProjectFile mode
        auto* worker = static_cast<ExportWorker*>(m_worker);
        if (AppState::instance().operationMode() == SerialStudio::ProjectFile)
          worker->m_templateFrame = DataModel::FrameBuilder::instance().frame();
        else
          DataModel::clear_frame(worker->m_templateFrame);
      }

      else {
        closeFile();
      }
    });
  connect(&IO::ConnectionManager::instance(), &IO::ConnectionManager::pausedChanged, this, [this] {
    if (IO::ConnectionManager::instance().paused())
      closeFile();
  });

  // Force-off when the app enters Console-only mode
  connect(&AppState::instance(), &AppState::operationModeChanged, this, [this] {
    if (AppState::instance().operationMode() == SerialStudio::ConsoleOnly && exportEnabled())
      setExportEnabled(false);
  });
}

/**
 * @brief Enables or disables CSV export, closing the file on disable.
 */
void CSV::Export::setExportEnabled(const bool enabled)
{
  // Refuse to enable while the app is in Console-only mode
  const bool allow = enabled && AppState::instance().operationMode() != SerialStudio::ConsoleOnly;

  // Close file first when disabling
  if (!allow && isOpen())
    closeFile();

  setConsumerEnabled(allow);
  Q_EMIT enabledChanged();
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

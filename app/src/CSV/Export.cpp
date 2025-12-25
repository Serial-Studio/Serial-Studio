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

#include "IO/Manager.h"
#include "CSV/Player.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Client.h"
#endif

#include <QDir>
#include <QDateTime>

//------------------------------------------------------------------------------
// ExportWorker implementation
//------------------------------------------------------------------------------

/**
 * @brief Constructs the CSV export worker.
 *
 * @param queue Pointer to the lock-free queue shared with Export.
 * @param exportEnabled Pointer to atomic flag indicating export state.
 * @param queueSize Pointer to atomic counter tracking queue size.
 */
CSV::ExportWorker::ExportWorker(
    moodycamel::ReaderWriterQueue<TimestampFrame> *queue,
    std::atomic<bool> *exportEnabled, std::atomic<size_t> *queueSize)
  : m_pendingFrames(queue)
  , m_exportEnabled(exportEnabled)
  , m_queueSize(queueSize)
{
  m_writeBuffer.reserve(kFlushThreshold * 2);
}

/**
 * @brief Destructor for the export worker.
 */
CSV::ExportWorker::~ExportWorker()
{
  closeFile();
}

/**
 * @brief Checks whether a CSV file is currently open.
 *
 * @return true if file is open, false otherwise.
 */
bool CSV::ExportWorker::isOpen() const
{
  return m_csvFile.isOpen();
}

/**
 * @brief Closes the currently open CSV file.
 *
 * Flushes any buffered data to disk, clears headers, and resets output.
 */
void CSV::ExportWorker::closeFile()
{
  if (!m_csvFile.isOpen())
    return;

  writeValues();
  m_csvFile.close();
  m_indexHeaderPairs.clear();
  m_textStream.setDevice(nullptr);
  Q_EMIT openChanged();
}

/**
 * @brief Writes all queued frames to the CSV file.
 *
 * Flushes data in the pending queue to disk using the output stream.
 * If no file is open, a new file is created before writing.
 */
void CSV::ExportWorker::writeValues()
{
  if (!m_exportEnabled->load(std::memory_order_relaxed))
    return;

  m_writeBuffer.clear();

  TimestampFrame frame;
  while (m_pendingFrames->try_dequeue(frame))
    m_writeBuffer.push_back(std::move(frame));

  const auto count = m_writeBuffer.size();
  if (count == 0)
    return;

  m_queueSize->fetch_sub(count, std::memory_order_relaxed);

  if (!m_csvFile.isOpen())
  {
    m_indexHeaderPairs = createCsvFile(m_writeBuffer.begin()->data);
    if (m_indexHeaderPairs.isEmpty())
      return;

    // Set reference timestamp from first frame (so timestamps start at 0)
    m_referenceTimestamp = m_writeBuffer.begin()->highResTimestamp;
  }

  for (const auto &i : m_writeBuffer)
  {
    // Calculate elapsed time since first frame
    const auto elapsed = i.highResTimestamp - m_referenceTimestamp;
    const auto nanoseconds
        = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    const double seconds = static_cast<double>(nanoseconds) / 1'000'000'000.0;

    // Write timestamp with nanosecond precision (9 decimal places)
    m_textStream << QString::number(seconds, 'f', 9) << QStringLiteral(",");

    QMap<int, QString> fieldValues;
    for (const auto &g : i.data.groups)
    {
      for (const auto &d : g.datasets)
        fieldValues[d.index] = d.value.simplified();
    }

    for (int j = 0; j < m_indexHeaderPairs.count(); ++j)
    {
      m_textStream << fieldValues.value(m_indexHeaderPairs[j].first, "");
      m_textStream << (j < m_indexHeaderPairs.count() - 1 ? "," : "\n");
    }
  }

  m_textStream.flush();
}

/**
 * @brief Creates a new CSV file and writes the header.
 *
 * Builds a sorted header based on dataset indices and opens the file
 * in the appropriate location.
 *
 * @param frame The frame used to extract header information.
 * @return List of index-title pairs for each dataset.
 */
QVector<QPair<int, QString>>
CSV::ExportWorker::createCsvFile(const JSON::Frame &frame)
{
  const auto dt = QDateTime::currentDateTime();
  const auto fileName = dt.toString("yyyy-MM-dd_HH-mm-ss") + ".csv";

  const auto subdir = Misc::WorkspaceManager::instance().path("CSV");
  const QString path = QString("%1/%2/").arg(subdir, frame.title);

  QDir dir(path);
  if (!dir.exists() && !dir.mkpath("."))
  {
    qWarning() << "Failed to create directory:" << path;
    return {};
  }

  m_csvFile.setFileName(dir.filePath(fileName));
  if (!m_csvFile.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    qWarning() << "Cannot open CSV file for writing:" << dir.filePath(fileName);
    return {};
  }

  m_textStream.setDevice(&m_csvFile);
  m_textStream.setGenerateByteOrderMark(true);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  m_textStream.setCodec("UTF-8");
#else
  m_textStream.setEncoding(QStringConverter::Utf8);
#endif

  QSet<int> seenIndexes;
  QVector<QPair<int, QString>> pairs;
  for (const auto &g : frame.groups)
  {
    for (const auto &d : g.datasets)
    {
      const int idx = d.index;
      if (seenIndexes.contains(idx))
        continue;

      seenIndexes.insert(idx);
      auto header = QString("%1/%2").arg(g.title, d.title).simplified();
      pairs.append(qMakePair(idx, header));
    }
  }

  std::sort(pairs.begin(), pairs.end(),
            [](const auto &a, const auto &b) { return a.first < b.first; });

  m_textStream << "RX Date/Time";
  for (const auto &pair : pairs)
    m_textStream << ',' << pair.second;

  m_textStream << '\n';

  Q_EMIT openChanged();
  return pairs;
}

//------------------------------------------------------------------------------
// Export constructor, destructor & singleton access functions
//------------------------------------------------------------------------------

/**
 * @brief Constructs the CSV export manager.
 *
 * Initializes the worker object on a background thread with a timer
 * for periodic data export.
 */
CSV::Export::Export()
  : m_exportEnabled(true)
  , m_isOpen(false)
  , m_queueSize(0)
  , m_worker(nullptr)
{
  m_worker = new ExportWorker(&m_pendingFrames, &m_exportEnabled, &m_queueSize);
  m_worker->moveToThread(&m_workerThread);

  connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
  connect(m_worker, &ExportWorker::openChanged, this,
          &Export::onWorkerOpenChanged, Qt::QueuedConnection);

  m_workerThread.start();

  auto *timer = new QTimer();
  timer->setInterval(1000);
  timer->setTimerType(Qt::PreciseTimer);
  timer->moveToThread(&m_workerThread);
  connect(timer, &QTimer::timeout, m_worker, &ExportWorker::writeValues);
  connect(&m_workerThread, &QThread::finished, timer, &QObject::deleteLater);
  QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection);
}

/**
 * @brief Destructor for the export manager.
 *
 * Ensures all data is flushed to disk and stops the background thread.
 */
CSV::Export::~Export()
{
  QMetaObject::invokeMethod(m_worker, "closeFile",
                            Qt::BlockingQueuedConnection);
  m_workerThread.quit();
  m_workerThread.wait();
}

/**
 * @brief Singleton access to the export manager.
 *
 * @return Reference to the global instance.
 */
CSV::Export &CSV::Export::instance()
{
  static Export singleton;
  return singleton;
}

//------------------------------------------------------------------------------
// State access functions
//------------------------------------------------------------------------------

/**
 * @brief Checks whether a CSV file is currently open.
 *
 * @return true if file is open, false otherwise.
 */
bool CSV::Export::isOpen() const
{
  return m_isOpen.load(std::memory_order_relaxed);
}

/**
 * @brief Checks whether CSV export is currently enabled.
 *
 * @return true if export is enabled, false otherwise.
 */
bool CSV::Export::exportEnabled() const
{
  return m_exportEnabled.load(std::memory_order_relaxed);
}

//------------------------------------------------------------------------------
// Public slots
//------------------------------------------------------------------------------

/**
 * @brief Closes the currently open CSV file.
 *
 * Delegates to the worker thread to ensure thread-safe file operations.
 */
void CSV::Export::closeFile()
{
  QMetaObject::invokeMethod(m_worker, "closeFile", Qt::QueuedConnection);
}

/**
 * @brief Called when the worker's open state changes.
 *
 * Updates the cached state and emits the signal on the main thread.
 */
void CSV::Export::onWorkerOpenChanged()
{
  m_isOpen.store(m_worker->isOpen(), std::memory_order_relaxed);
  Q_EMIT openChanged();
}

/**
 * @brief Sets up connections with other modules.
 *
 * Links to IO and JSON frame events to control export behavior.
 */
void CSV::Export::setupExternalConnections()
{
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this,
          &Export::closeFile);
  connect(&IO::Manager::instance(), &IO::Manager::pausedChanged, this, [this] {
    if (IO::Manager::instance().paused())
      closeFile();
  });
}

/**
 * @brief Enables or disables CSV export.
 *
 * When disabling, writes remaining data to disk and closes the file.
 *
 * @param enabled True to enable export, false to disable.
 */
void CSV::Export::setExportEnabled(const bool enabled)
{
  if (!enabled && isOpen())
    closeFile();

  m_exportEnabled.store(enabled, std::memory_order_relaxed);
  Q_EMIT enabledChanged();
}

//------------------------------------------------------------------------------
// Hotpath data processing
//------------------------------------------------------------------------------

/**
 * @brief Registers a new data frame for export.
 *
 * Pushes the frame into the pending queue for async export if conditions are
 * met. Triggers an early flush when the queue exceeds the threshold to prevent
 * unbounded memory growth during high-frequency data reception.
 *
 * @param frame The data frame to export.
 */
void CSV::Export::hotpathTxFrame(const JSON::Frame &frame)
{
  if (!exportEnabled() || CSV::Player::instance().isOpen())
    return;

#ifdef BUILD_COMMERCIAL
  if (!IO::Manager::instance().isConnected()
      && !(MQTT::Client::instance().isConnected()
           && MQTT::Client::instance().isSubscriber()))
    return;
#else
  if (!IO::Manager::instance().isConnected())
    return;
#endif

  if (!m_pendingFrames.enqueue(TimestampFrame(JSON::Frame(frame))))
  {
    qWarning() << "CSV Export: Dropping frame (queue full)";
    return;
  }

  const auto size = m_queueSize.fetch_add(1, std::memory_order_relaxed) + 1;
  if (size >= kFlushThreshold)
    QMetaObject::invokeMethod(m_worker, "writeValues", Qt::QueuedConnection);
}

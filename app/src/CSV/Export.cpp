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
// Constructor, destructor & singleton access functions
//------------------------------------------------------------------------------

/**
 * @brief Constructs the CSV export manager.
 *
 * Initializes the write buffer, sets up the background worker thread
 * and starts a timer for periodic data export.
 */
CSV::Export::Export()
  : m_exportEnabled(true)
  , m_workerTimer(new QTimer())
{
  // Pre-allocate memory for the write buffer
  m_writeBuffer.reserve(m_pendingFrames.max_capacity());

  // Configure the data write timer
  m_workerTimer->setInterval(1000);
  m_workerTimer->setTimerType(Qt::PreciseTimer);
  m_workerTimer->moveToThread(&m_workerThread);
  connect(m_workerTimer, &QTimer::timeout, this, &Export::writeValues,
          Qt::QueuedConnection);

  // Start the data writting thread
  m_workerThread.start();
  connect(&m_workerThread, &QThread::finished, m_workerTimer,
          &QObject::deleteLater);

  // Start the data writting timer
  QMetaObject::invokeMethod(m_workerTimer, "start", Qt::QueuedConnection);
}

/**
 * @brief Destructor for the export manager.
 *
 * Ensures all data is flushed to disk, stops the background thread
 * and cleans up the timer.
 */
CSV::Export::~Export()
{
  // Close the file
  closeFile();

  // Stop the worker thread
  if (m_workerTimer)
  {
    QMetaObject::invokeMethod(m_workerTimer, "stop", Qt::QueuedConnection);
    QMetaObject::invokeMethod(m_workerTimer, "deleteLater",
                              Qt::QueuedConnection);
    m_workerTimer = nullptr;
  }

  // Wait for the worker thread to finish before quitting
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
  return m_csvFile.isOpen();
}

/**
 * @brief Checks whether CSV export is currently enabled.
 *
 * @return true if export is enabled, false otherwise.
 */
bool CSV::Export::exportEnabled() const
{
  return m_exportEnabled;
}

//------------------------------------------------------------------------------
// Public slots
//------------------------------------------------------------------------------

/**
 * @brief Closes the currently open CSV file.
 *
 * Flushes any buffered data to disk, clears headers, and resets output.
 */
void CSV::Export::closeFile()
{
  // No point in closing a file that is not open...
  if (!isOpen())
    return;

  // Write pending data to disk
  writeValues();

  // Close the file & reset the status
  m_csvFile.close();
  m_indexHeaderPairs.clear();
  m_textStream.setDevice(nullptr);

  // Update UI
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
  connect(&IO::Manager::instance(), &IO::Manager::pausedChanged, this,
          [=, this] {
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
  // Write all pending data to disk & close file
  if (!enabled && isOpen())
    closeFile();

  // Update the enabled status
  m_exportEnabled = enabled;
  Q_EMIT enabledChanged();
}

//------------------------------------------------------------------------------
// Hotpath data processing
//------------------------------------------------------------------------------

/**
 * @brief Registers a new data frame for export.
 *
 * Pushes the frame into the pending queue for async export if conditions are
 * met.
 *
 * @param frame The data frame to export.
 */
void CSV::Export::hotpathTxFrame(const JSON::Frame &frame)
{
  // Skip if export is disabled, frame is invalid or user is playing a CSV file
  if (!exportEnabled() || CSV::Player::instance().isOpen())
    return;

    // Skip if not connected to a device
#ifdef BUILD_COMMERCIAL
  if (!IO::Manager::instance().isConnected()
      && !(MQTT::Client::instance().isConnected()
           && MQTT::Client::instance().isSubscriber()))
    return;
#else
  if (!IO::Manager::instance().isConnected())
    return;
#endif

  // Add frame to pending frame queue
  if (!m_pendingFrames.enqueue(TimestampFrame(JSON::Frame(frame))))
    qWarning() << "CSV Export: Dropping frame (queue full)";
}

//------------------------------------------------------------------------------
// CSV data processing
//------------------------------------------------------------------------------

/**
 * @brief Writes all queued frames to the CSV file.
 *
 * Flushes data in `m_pendingFrames` to disk using the output stream.
 * If no file is open, a new file is created before writing.
 */
void CSV::Export::writeValues()
{
  // Export disabled, abort
  if (!exportEnabled())
    return;

  // Reset the write buffer
  m_writeBuffer.clear();

  // Read frames in queue
  TimestampFrame frame;
  while (m_pendingFrames.try_dequeue(frame))
    m_writeBuffer.push_back(std::move(frame));

  // Nothing to write, abort
  if (m_writeBuffer.size() <= 0)
    return;

  // File deleted, create new file
  if (!isOpen())
  {
    m_indexHeaderPairs = createCsvFile(m_writeBuffer.begin()->data);
    if (m_indexHeaderPairs.isEmpty())
      return;
  }

  // Write every frame to the CSV file
  for (const auto &i : m_writeBuffer)
  {
    // Write RX date/time
    const auto format = QStringLiteral("yyyy/MM/dd HH:mm:ss::zzz");
    m_textStream << i.rxDateTime.toString(format) << QStringLiteral(",");

    // Obtain a set of unique dataset values (based on dataset index)
    QMap<int, QString> fieldValues;
    for (const auto &g : i.data.groups)
    {
      for (const auto &d : g.datasets)
        fieldValues[d.index] = d.value.simplified();
    }

    // Write data to output stream
    for (int j = 0; j < m_indexHeaderPairs.count(); ++j)
    {
      m_textStream << fieldValues.value(m_indexHeaderPairs[j].first, "");
      m_textStream << (j < m_indexHeaderPairs.count() - 1 ? "," : "\n");
    }
  }

  // Flush the output stream to the disk
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
CSV::Export::createCsvFile(const JSON::Frame &frame)
{
  // Get filename based on date time
  const auto dt = QDateTime::currentDateTime();
  const auto fileName = dt.toString("yyyy-MM-dd_HH-mm-ss") + ".csv";

  // Get file path
  const auto subdir = Misc::WorkspaceManager::instance().path("CSV");
  const QString path = QString("%1/%2/").arg(subdir, frame.title);

  // Create the CSVs directory if needed
  QDir dir(path);
  if (!dir.exists() && !dir.mkpath("."))
  {
    qWarning() << "Failed to create directory:" << path;
    return {};
  }

  // Open the CSV file for writting
  m_csvFile.setFileName(dir.filePath(fileName));
  if (!m_csvFile.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    Misc::Utilities::showMessageBox(tr("CSV File Error"),
                                    tr("Cannot open CSV file for writing!"),
                                    QMessageBox::Critical);
    closeFile();
    return {};
  }

  // Configure the output stream to use the file, and set data mode to UTF-8
  m_textStream.setDevice(&m_csvFile);
  m_textStream.setGenerateByteOrderMark(true);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  m_textStream.setCodec("UTF-8");
#else
  m_textStream.setEncoding(QStringConverter::Utf8);
#endif

  // Create a list of pairs that relate dataset indexes to header titles
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

  // Sort the pairs by their dataset index
  std::sort(pairs.begin(), pairs.end(),
            [](const auto &a, const auto &b) { return a.first < b.first; });

  // Write CSV header
  m_textStream << "RX Date/Time";
  for (const auto &pair : pairs)
    m_textStream << ',' << pair.second;

  // Add EOL to header line
  m_textStream << '\n';

  // Update user interface & return the data model
  Q_EMIT openChanged();
  return pairs;
}

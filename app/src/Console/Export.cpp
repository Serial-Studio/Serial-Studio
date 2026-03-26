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

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QUrl>

#include "Misc/Utilities.h"

#ifdef BUILD_COMMERCIAL
#  include "AppState.h"
#  include "Console/Handler.h"
#  include "DataModel/ProjectModel.h"
#  include "IO/ConnectionManager.h"
#  include "Licensing/CommercialToken.h"
#  include "Licensing/LemonSqueezy.h"
#  include "Misc/WorkspaceManager.h"
#  include "SerialStudio.h"
#endif

//--------------------------------------------------------------------------------------------------
// ExportWorker implementation
//--------------------------------------------------------------------------------------------------

#ifdef BUILD_COMMERCIAL

/**
 * @brief Destructor - closes file
 */
Console::ExportWorker::~ExportWorker() = default;

/**
 * @brief Returns true if at least one device file is open
 */
bool Console::ExportWorker::isResourceOpen() const
{
  for (const auto& [id, state] : m_deviceFiles)
    if (state.file && state.file->isOpen())
      return true;

  return false;
}

/**
 * @brief Processes a batch of console data items, routing to per-device files.
 */
void Console::ExportWorker::processItems(const std::vector<ExportDataPtr>& items)
{
  // No items, abort
  if (items.empty())
    return;

  // No device connected, abort
  if (!IO::ConnectionManager::instance().isConnected())
    return;

  // Route each item to its device file
  for (const auto& dataPtr : items) {
    const int devId = dataPtr->deviceId;

    // Ensure file exists for this device
    auto it = m_deviceFiles.find(devId);
    if (it == m_deviceFiles.end() || !it->second.file || !it->second.file->isOpen())
      createFile(devId);

    it = m_deviceFiles.find(devId);
    if (it != m_deviceFiles.end() && it->second.stream && it->second.stream->device()) {
      *it->second.stream << dataPtr->data;
      it->second.stream->flush();
    }
  }
}

/**
 * @brief Closes all per-device output files.
 */
void Console::ExportWorker::closeResources()
{
  bool wasOpen = isResourceOpen();
  for (auto& [id, state] : m_deviceFiles) {
    if (state.file && state.file->isOpen()) {
      state.stream.reset();
      state.file->close();
    }
  }

  m_deviceFiles.clear();

  if (wasOpen)
    Q_EMIT resourceOpenChanged();
}

/**
 * @brief Creates a new console log file for the given device.
 * @param deviceId Device to create a file for (-1 for single-device mode).
 */
void Console::ExportWorker::createFile(int deviceId)
{
  const auto& token = Licensing::CommercialToken::current();
  if (!token.isValid() || token.featureTier() == Licensing::FeatureTier::None)
    return;

  // Close existing file for this device
  auto it = m_deviceFiles.find(deviceId);
  if (it != m_deviceFiles.end()) {
    it->second.stream.reset();
    if (it->second.file)
      it->second.file->close();

    m_deviceFiles.erase(it);
  }

  // Obtain file name with optional device suffix
  const auto dateTime = QDateTime::currentDateTime();
  auto fileName       = dateTime.toString(QStringLiteral("yyyy_MMM_dd HH_mm_ss"));
  if (deviceId >= 0)
    fileName += QStringLiteral("_device%1").arg(deviceId);

  fileName += QStringLiteral(".txt");

  // Derive project subdirectory name (same pattern as CSV/MDF4 export)
  const auto opMode        = AppState::instance().operationMode();
  const auto& projectTitle = DataModel::ProjectModel::instance().title();
  QString subdirName;
  if (opMode == SerialStudio::ProjectFile && !projectTitle.isEmpty())
    subdirName = projectTitle;
  else if (opMode == SerialStudio::QuickPlot)
    subdirName = QStringLiteral("Quick Plot");
  else
    subdirName = QStringLiteral("Untitled");

  // Obtain directory where to write file
  QDir dir(Misc::WorkspaceManager::instance().path("Console"));
  if (!dir.exists(subdirName))
    dir.mkpath(subdirName);

  dir.cd(subdirName);

  // Create per-device state
  auto& state = m_deviceFiles[deviceId];
  state.file  = std::make_unique<QFile>(dir.filePath(fileName));

  if (!state.file->open(QIODeviceBase::WriteOnly | QIODevice::Text)) {
    Misc::Utilities::showMessageBox(QObject::tr("Console Output File Error"),
                                    QObject::tr("Cannot open file for writing!"),
                                    QMessageBox::Critical);
    m_deviceFiles.erase(deviceId);
    return;
  }

  // Configure the output stream
  state.stream = std::make_unique<QTextStream>(state.file.get());
  state.stream->setGenerateByteOrderMark(true);
  state.stream->setEncoding(QStringConverter::Utf8);
  Q_EMIT resourceOpenChanged();
}

#endif

//--------------------------------------------------------------------------------------------------
// Export implementation
//--------------------------------------------------------------------------------------------------

/**
 * Constructor function, configures the path in which Serial Studio shall
 * automatically write generated console log files.
 */
Console::Export::Export()
#ifdef BUILD_COMMERCIAL
  : DataModel::FrameConsumer<ExportDataPtr>(
      {.queueCapacity = 8192, .flushThreshold = 1024, .timerIntervalMs = 1000})
  , m_isOpen(false)
  , m_exportEnabled(false)
#else
  : m_isOpen(false), m_exportEnabled(false)
#endif
{
#ifdef BUILD_COMMERCIAL
  // Initialize the internal worker thread
  initializeWorker();

  // Change open status when the worker updates its internal status
  connect(m_worker,
          &ExportWorker::resourceOpenChanged,
          this,
          &Export::onWorkerOpenChanged,
          Qt::QueuedConnection);

  // Disable console export if user de-activates Serial Studio
  connect(&Licensing::LemonSqueezy::instance(),
          &Licensing::LemonSqueezy::activatedChanged,
          this,
          [=, this] {
            if (exportEnabled() && !Licensing::CommercialToken::current().isValid())
              setExportEnabled(false);
          });
#endif

  setExportEnabled(m_settings.value("ConsoleExport", false).toBool());
}

/**
 * Close file & finnish write-operations before destroying the class.
 */
Console::Export::~Export() = default;

#ifdef BUILD_COMMERCIAL
/**
 * @brief Creates the console export worker instance.
 */
DataModel::FrameConsumerWorkerBase* Console::Export::createWorker()
{
  return new ExportWorker(&m_pendingQueue, &m_consumerEnabled, &m_queueSize);
}
#endif

/**
 * Returns a pointer to the only instance of this class.
 */
Console::Export& Console::Export::instance()
{
  static Export instance;
  return instance;
}

/**
 * Returns @c true if the console output file is open.
 */
bool Console::Export::isOpen() const
{
#ifdef BUILD_COMMERCIAL
  return m_isOpen.load(std::memory_order_relaxed);
#else
  return false;
#endif
}

/**
 * Returns @c true if console log export is enabled.
 */
bool Console::Export::exportEnabled() const
{
#ifdef BUILD_COMMERCIAL
  return consumerEnabled();
#else
  return false;
#endif
}

/**
 * Write all remaining console data & close the output file.
 *
 * Flushes any remaining data in the queue before closing to prevent
 * creating small trailing files.
 */
void Console::Export::closeFile()
{
#ifdef BUILD_COMMERCIAL
  auto* worker = static_cast<ExportWorker*>(m_worker);
  QMetaObject::invokeMethod(worker, "close", Qt::QueuedConnection);
#endif
}

/**
 * Configures the signal/slot connections with the modules of the application
 * that this module depends upon.
 */
void Console::Export::setupExternalConnections()
{
#ifdef BUILD_COMMERCIAL
  connect(&Console::Handler::instance(),
          &Console::Handler::deviceDataReady,
          this,
          &Console::Export::registerData);
  connect(&IO::ConnectionManager::instance(),
          &IO::ConnectionManager::connectedChanged,
          this,
          &Console::Export::closeFile);
#endif
}

/**
 * Enables or disables data export.
 */
void Console::Export::setExportEnabled(const bool enabled)
{
#ifdef BUILD_COMMERCIAL
  // Update export status only if activated
  const auto& tk = Licensing::CommercialToken::current();
  if (tk.isValid() && tk.featureTier() >= Licensing::FeatureTier::Trial) {
    if (!enabled && isOpen())
      closeFile();

    setConsumerEnabled(enabled);
    m_settings.setValue("ConsoleExport", enabled);
    Q_EMIT enabledChanged();
    return;
  }

  setConsumerEnabled(false);
#endif

  // Close file and disable export
  closeFile();
  m_exportEnabled.store(false, std::memory_order_relaxed);
  m_settings.setValue("ConsoleExport", false);
  Q_EMIT enabledChanged();

  // If we reach here, either Serial Studio is not activated, or it is a GPLv3 build
  if (enabled)
    Misc::Utilities::showMessageBox(
      tr("Console Export is a Pro feature."),
      tr("This feature requires a license. Please purchase one to enable console export."));
}

/**
 * @brief Appends console data from a specific device to the output buffer.
 * @param deviceId Source device identifier.
 * @param data     Console text to export.
 */
void Console::Export::registerData(int deviceId, QStringView data)
{
#ifdef BUILD_COMMERCIAL
  if (exportEnabled() && !data.isEmpty() && !SerialStudio::isAnyPlayerOpen())
    enqueueData(std::make_shared<ExportData>(deviceId, QString(data)));
#else
  (void)deviceId;
  (void)data;
#endif
}

/**
 * Called when the worker thread changes the file open state.
 */
#ifdef BUILD_COMMERCIAL
void Console::Export::onWorkerOpenChanged()
{
  auto* worker = static_cast<ExportWorker*>(m_worker);
  m_isOpen.store(worker->isResourceOpen(), std::memory_order_relaxed);
  Q_EMIT openChanged();
}
#endif

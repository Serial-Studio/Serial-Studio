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
 * @brief Default destructor.
 */
Console::ExportWorker::~ExportWorker() = default;

/**
 * @brief Returns true if at least one device file is open.
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
  if (items.empty())
    return;

  if (!IO::ConnectionManager::instance().isConnected())
    return;

  for (const auto& dataPtr : items) {
    const int devId = dataPtr->deviceId;
    auto it         = m_deviceFiles.find(devId);
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
  // Require a valid license for export
  const auto& token = Licensing::CommercialToken::current();
  if (!token.isValid() || !SS_LICENSE_GUARD()
      || token.featureTier() < Licensing::FeatureTier::Hobbyist)
    return;

  // Close any existing file for this device
  auto it = m_deviceFiles.find(deviceId);
  if (it != m_deviceFiles.end()) {
    it->second.stream.reset();
    if (it->second.file)
      it->second.file->close();

    m_deviceFiles.erase(it);
  }

  // Build filename with optional device suffix
  const auto dateTime = QDateTime::currentDateTime();
  auto fileName       = dateTime.toString(QStringLiteral("yyyy_MMM_dd HH_mm_ss"));
  if (deviceId >= 0)
    fileName += QStringLiteral("_device%1").arg(deviceId);

  fileName += QStringLiteral(".txt");

  // Derive project subdirectory name
  const auto opMode        = AppState::instance().operationMode();
  const auto& projectTitle = DataModel::ProjectModel::instance().title();
  QString subdirName;
  if (opMode == SerialStudio::ProjectFile && !projectTitle.isEmpty())
    subdirName = projectTitle;
  else if (opMode == SerialStudio::QuickPlot)
    subdirName = QStringLiteral("Quick Plot");
  else
    subdirName = QStringLiteral("Untitled");

  // Ensure output directory exists
  QDir dir(Misc::WorkspaceManager::instance().path("Console"));
  if (!dir.exists(subdirName))
    dir.mkpath(subdirName);

  dir.cd(subdirName);

  // Open the output file
  auto& state = m_deviceFiles[deviceId];
  state.file  = std::make_unique<QFile>(dir.filePath(fileName));

  if (!state.file->open(QIODeviceBase::WriteOnly | QIODevice::Text)) {
    Misc::Utilities::showMessageBox(QObject::tr("Console Output File Error"),
                                    QObject::tr("Cannot open file for writing!"),
                                    QMessageBox::Critical);
    m_deviceFiles.erase(deviceId);
    return;
  }

  // Configure UTF-8 output stream
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
 * @brief Constructs the console export singleton.
 */
Console::Export::Export()
#ifdef BUILD_COMMERCIAL
  : DataModel::FrameConsumer<ExportDataPtr>(
      {.queueCapacity = 8192, .flushThreshold = 1024, .timerIntervalMs = 1000})
  , m_isOpen(false)
  , m_exportEnabled(false)
  , m_persistSettings(true)
#else
  : m_isOpen(false), m_exportEnabled(false), m_persistSettings(true)
#endif
{
#ifdef BUILD_COMMERCIAL
  // Initialize worker and track open state
  initializeWorker();
  connect(m_worker,
          &ExportWorker::resourceOpenChanged,
          this,
          &Export::onWorkerOpenChanged,
          Qt::QueuedConnection);

  // Disable export on license deactivation
  connect(&Licensing::LemonSqueezy::instance(),
          &Licensing::LemonSqueezy::activatedChanged,
          this,
          [=, this] {
            if (exportEnabled()
                && (!Licensing::CommercialToken::current().isValid() || !SS_LICENSE_GUARD()))
              setExportEnabled(false);
          });
#endif

  setExportEnabled(m_settings.value("ConsoleExport", false).toBool());
}

/**
 * @brief Default destructor.
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
 * @brief Returns the singleton instance.
 */
Console::Export& Console::Export::instance()
{
  static Export instance;
  return instance;
}

/**
 * @brief Returns true if the console output file is open.
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
 * @brief Returns true if console log export is enabled.
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
 * @brief Flushes remaining console data and closes the output file.
 */
void Console::Export::closeFile()
{
#ifdef BUILD_COMMERCIAL
  auto* worker = static_cast<ExportWorker*>(m_worker);
  QMetaObject::invokeMethod(worker, "close", Qt::QueuedConnection);
#endif
}

/**
 * @brief Configures signal/slot connections with dependent modules.
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
 * @brief Toggles whether export-enabled changes get written to QSettings.
 */
void Console::Export::setSettingsPersistent(const bool persistent)
{
  m_persistSettings = persistent;
}

/**
 * @brief Enables or disables data export.
 */
void Console::Export::setExportEnabled(const bool enabled)
{
#ifdef BUILD_COMMERCIAL
  const auto& tk = Licensing::CommercialToken::current();
  if (tk.isValid() && SS_LICENSE_GUARD() && tk.featureTier() >= Licensing::FeatureTier::Hobbyist) {
    if (!enabled && isOpen())
      closeFile();

    setConsumerEnabled(enabled);
    if (m_persistSettings)
      m_settings.setValue("ConsoleExport", enabled);
    Q_EMIT enabledChanged();
    return;
  }

  setConsumerEnabled(false);
#endif

  closeFile();
  m_exportEnabled.store(false, std::memory_order_relaxed);
  if (m_persistSettings)
    m_settings.setValue("ConsoleExport", false);
  Q_EMIT enabledChanged();

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
 * @brief Called when the worker thread changes the file open state.
 */
#ifdef BUILD_COMMERCIAL
void Console::Export::onWorkerOpenChanged()
{
  auto* worker = static_cast<ExportWorker*>(m_worker);
  m_isOpen.store(worker->isResourceOpen(), std::memory_order_relaxed);
  Q_EMIT openChanged();
}
#endif

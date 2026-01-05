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

#include <QDir>
#include <QUrl>
#include <QTimer>
#include <QFileInfo>
#include <QApplication>
#include <QDesktopServices>

#include "Misc/Utilities.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Manager.h"
#  include "SerialStudio.h"
#  include "Console/Handler.h"
#  include "Misc/WorkspaceManager.h"
#  include "Licensing/LemonSqueezy.h"
#endif

//------------------------------------------------------------------------------
// ExportWorker implementation
//------------------------------------------------------------------------------

#ifdef BUILD_COMMERCIAL

/**
 * @brief Destructor - closes file
 */
Console::ExportWorker::~ExportWorker() = default;

/**
 * @brief Returns true if file is open
 */
bool Console::ExportWorker::isResourceOpen() const
{
  return m_file.isOpen();
}

/**
 * @brief Processes a batch of console data items
 */
void Console::ExportWorker::processItems(
    const std::vector<ExportDataPtr> &items)
{
  if (items.empty())
    return;

  if (!IO::Manager::instance().isConnected())
    return;

  if (!isResourceOpen())
    createFile();

  if (m_textStream.device())
  {
    for (const auto &dataPtr : items)
      m_textStream << dataPtr->data;

    m_textStream.flush();
  }
}

/**
 * @brief Closes the output file
 */
void Console::ExportWorker::closeResources()
{
  if (isResourceOpen())
  {
    m_file.close();
    m_textStream.setDevice(nullptr);
    Q_EMIT resourceOpenChanged();
  }
}

/**
 * @brief Creates a new console log file
 */
void Console::ExportWorker::createFile()
{
  if (SerialStudio::activated())
  {
    if (isResourceOpen())
      closeResources();

    const auto dateTime = QDateTime::currentDateTime();
    const auto fileName
        = dateTime.toString(QStringLiteral("yyyy_MMM_dd HH_mm_ss"))
          + QStringLiteral(".txt");

    QDir dir(Misc::WorkspaceManager::instance().path("Console"));

    m_file.setFileName(dir.filePath(fileName));
    if (!m_file.open(QIODeviceBase::WriteOnly | QIODevice::Text))
    {
      Misc::Utilities::showMessageBox(
          QObject::tr("Console Output File Error"),
          QObject::tr("Cannot open file for writing!"), QMessageBox::Critical);
      closeResources();
      return;
    }

    m_textStream.setDevice(&m_file);
    m_textStream.setGenerateByteOrderMark(true);
    m_textStream.setEncoding(QStringConverter::Utf8);

    Q_EMIT resourceOpenChanged();
  }
}

#endif

//------------------------------------------------------------------------------
// Export implementation
//------------------------------------------------------------------------------

/**
 * Constructor function, configures the path in which Serial Studio shall
 * automatically write generated console log files.
 */
Console::Export::Export()
#ifdef BUILD_COMMERCIAL
  : DataModel::FrameConsumer<ExportDataPtr>({.queueCapacity = 8192,
                                             .flushThreshold = 1024,
                                             .timerIntervalMs = 1000})
  , m_isOpen(false)
  , m_exportEnabled(false)
#else
  : m_isOpen(false)
  , m_exportEnabled(false)
#endif
{
#ifdef BUILD_COMMERCIAL
  initializeWorker();
  connect(m_worker, &ExportWorker::resourceOpenChanged, this,
          &Export::onWorkerOpenChanged, Qt::QueuedConnection);

  connect(&Licensing::LemonSqueezy::instance(),
          &Licensing::LemonSqueezy::activatedChanged, this, [=, this] {
            if (exportEnabled() && !SerialStudio::activated())
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
DataModel::FrameConsumerWorkerBase *Console::Export::createWorker()
{
  return new ExportWorker(&m_pendingQueue, &m_consumerEnabled, &m_queueSize);
}
#endif

/**
 * Returns a pointer to the only instance of this class.
 */
Console::Export &Console::Export::instance()
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
  auto *worker = static_cast<ExportWorker *>(m_worker);
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
  connect(&Console::Handler::instance(), &Console::Handler::displayString, this,
          &Console::Export::registerData);
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this,
          &Console::Export::closeFile);
#endif
}

/**
 * Enables or disables data export.
 */
void Console::Export::setExportEnabled(const bool enabled)
{
#ifdef BUILD_COMMERCIAL
  if (SerialStudio::activated())
  {
    if (!enabled && isOpen())
      closeFile();

    setConsumerEnabled(enabled);
    m_settings.setValue("ConsoleExport", enabled);
    Q_EMIT enabledChanged();
    return;
  }

  closeFile();
  setConsumerEnabled(false);
  m_settings.setValue("ConsoleExport", false);
  Q_EMIT enabledChanged();
#else
  closeFile();
  m_exportEnabled.store(false, std::memory_order_relaxed);
  m_settings.setValue("ConsoleExport", false);
  Q_EMIT enabledChanged();
#endif

  if (enabled)
    Misc::Utilities::showMessageBox(
        tr("Console Export is a Pro feature."),
        tr("This feature requires a license. Please "
           "purchase one to enable console export."));
}

/**
 * Appends the given console data to the output buffer.
 */
void Console::Export::registerData(QStringView data)
{
#ifdef BUILD_COMMERCIAL
  if (exportEnabled() && !data.isEmpty() && !SerialStudio::isAnyPlayerOpen())
    enqueueData(std::make_shared<ExportData>(QString(data)));
#else
  (void)data;
#endif
}

/**
 * Called when the worker thread changes the file open state.
 */
#ifdef BUILD_COMMERCIAL
void Console::Export::onWorkerOpenChanged()
{
  auto *worker = static_cast<ExportWorker *>(m_worker);
  m_isOpen.store(worker->isResourceOpen(), std::memory_order_relaxed);
  Q_EMIT openChanged();
}
#endif

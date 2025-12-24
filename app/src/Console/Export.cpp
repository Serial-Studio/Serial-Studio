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
#  include "Console/Handler.h"
#  include "Misc/WorkspaceManager.h"
#  include "Licensing/LemonSqueezy.h"
#endif

//------------------------------------------------------------------------------
// ExportWorker implementation
//------------------------------------------------------------------------------

#ifdef BUILD_COMMERCIAL

/**
 * @brief Constructs the console export worker
 */
Console::ExportWorker::ExportWorker(
    moodycamel::ReaderWriterQueue<Console::ExportData> *queue,
    std::atomic<bool> *exportEnabled, std::atomic<size_t> *queueSize)
  : m_pendingData(queue)
  , m_exportEnabled(exportEnabled)
  , m_queueSize(queueSize)
{
  m_writeBuffer.reserve(1024);
}

/**
 * @brief Destructor - closes file
 */
Console::ExportWorker::~ExportWorker()
{
  closeFile();
}

/**
 * @brief Returns true if file is open
 */
bool Console::ExportWorker::isOpen() const
{
  return m_file.isOpen();
}

/**
 * @brief Writes all queued console data to file
 */
void Console::ExportWorker::writeValues()
{
  if (!m_exportEnabled->load(std::memory_order_relaxed))
    return;

  if (!IO::Manager::instance().isConnected())
    return;

  m_writeBuffer.clear();

  ExportData item;
  while (m_pendingData->try_dequeue(item))
    m_writeBuffer.push_back(std::move(item));

  const auto count = m_writeBuffer.size();
  if (count == 0)
    return;

  m_queueSize->fetch_sub(count, std::memory_order_relaxed);

  if (!isOpen())
    createFile();

  if (m_textStream.device())
  {
    for (const auto &data : m_writeBuffer)
      m_textStream << data.data;

    m_textStream.flush();
  }
}

/**
 * @brief Closes the output file
 */
void Console::ExportWorker::closeFile()
{
  if (isOpen())
  {
    writeValues();
    m_file.close();
    m_textStream.setDevice(nullptr);
    Q_EMIT openChanged();
  }
}

/**
 * @brief Creates a new console log file
 */
void Console::ExportWorker::createFile()
{
  if (SerialStudio::activated())
  {
    if (isOpen())
      closeFile();

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
      closeFile();
      return;
    }

    m_textStream.setDevice(&m_file);
    m_textStream.setGenerateByteOrderMark(true);
    m_textStream.setEncoding(QStringConverter::Utf8);

    Q_EMIT openChanged();
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
  : m_isOpen(false)
  , m_exportEnabled(false)
#ifdef BUILD_COMMERCIAL
  , m_worker(nullptr)
  , m_queueSize(0)
#endif
{
#ifdef BUILD_COMMERCIAL
  m_worker = new ExportWorker(&m_pendingData, &m_exportEnabled, &m_queueSize);
  m_worker->moveToThread(&m_workerThread);

  connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
  connect(m_worker, &ExportWorker::openChanged, this,
          &Export::onWorkerOpenChanged);

  m_workerThread.start();

  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, m_worker, &ExportWorker::writeValues);
  timer->start(1000);

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
Console::Export::~Export()
{
#ifdef BUILD_COMMERCIAL
  m_workerThread.quit();
  m_workerThread.wait();
#endif
}

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
  return m_exportEnabled.load(std::memory_order_relaxed);
#else
  return false;
#endif
}

/**
 * Write all remaining console data & close the output file.
 */
void Console::Export::closeFile()
{
#ifdef BUILD_COMMERCIAL
  if (m_worker)
    QMetaObject::invokeMethod(m_worker, &ExportWorker::closeFile,
                              Qt::QueuedConnection);
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
    m_exportEnabled.store(enabled, std::memory_order_relaxed);
    Q_EMIT enabledChanged();

    if (!enabled && isOpen())
      closeFile();

    m_settings.setValue("Export", enabled);
    return;
  }
#endif

  closeFile();
  m_exportEnabled.store(false, std::memory_order_relaxed);
  m_settings.setValue("Export", false);
  Q_EMIT enabledChanged();

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
  if (!data.isEmpty() && exportEnabled())
  {
    if (m_queueSize.load(std::memory_order_relaxed) < kExportQueueCapacity)
    {
      m_pendingData.enqueue(ExportData(QString(data)));
      m_queueSize.fetch_add(1, std::memory_order_relaxed);
    }
  }
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
  const bool workerIsOpen = m_worker ? m_worker->isOpen() : false;
  m_isOpen.store(workerIsOpen, std::memory_order_relaxed);
  Q_EMIT openChanged();
}
#endif

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
#include <QTimer>
#include <QDateTime>

#include "SerialStudio.h"
#include "Misc/Utilities.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Manager.h"
#  include "MDF4/Player.h"
#  include "Misc/WorkspaceManager.h"
#  include "Licensing/LemonSqueezy.h"

#  include <mdf/mdffactory.h>
#  include <mdf/mdfwriter.h>
#  include <mdf/ichannel.h>
#  include <mdf/ichannelgroup.h>
#  include <mdf/idatagroup.h>
#  include <mdf/iheader.h>
#endif

//------------------------------------------------------------------------------
// ExportWorker implementation
//------------------------------------------------------------------------------

#ifdef BUILD_COMMERCIAL

/**
 * @brief Constructs the MDF4 export worker
 */
MDF4::ExportWorker::ExportWorker(
    moodycamel::ReaderWriterQueue<MDF4::TimestampFrame> *queue,
    std::atomic<bool> *exportEnabled, std::atomic<size_t> *queueSize)
  : m_fileOpen(false)
  , m_writer(nullptr)
  , m_masterTimeChannel(nullptr)
  , m_pendingFrames(queue)
  , m_exportEnabled(exportEnabled)
  , m_queueSize(queueSize)
{
  m_writeBuffer.reserve(kMDF4FlushThreshold * 2);
}

/**
 * @brief Destructor - closes file
 */
MDF4::ExportWorker::~ExportWorker()
{
  closeFile();
}

/**
 * @brief Returns true if file is open
 */
bool MDF4::ExportWorker::isOpen() const
{
  return m_fileOpen;
}

/**
 * @brief Writes all queued frames to the MDF4 file
 */
void MDF4::ExportWorker::writeValues()
{
  if (!m_exportEnabled->load(std::memory_order_relaxed))
    return;

  if (!IO::Manager::instance().isConnected())
    return;

  m_writeBuffer.clear();

  TimestampFrame item;
  while (m_pendingFrames->try_dequeue(item))
    m_writeBuffer.push_back(std::move(item));

  const auto count = m_writeBuffer.size();
  if (count == 0)
    return;

  m_queueSize->fetch_sub(count, std::memory_order_relaxed);

  if (!isOpen() && !m_writeBuffer.empty())
    createFile(m_writeBuffer.front().data);

  if (!isOpen() || !m_writer)
    return;

  for (const auto &frame : m_writeBuffer)
  {
    const auto timestamp = frame.rxDateTime.toMSecsSinceEpoch() / 1000.0;

    for (const auto &group : frame.data.groups)
    {
      auto it = m_groupMap.find(group.groupId);
      if (it == m_groupMap.end())
        continue;

      auto &info = it->second;
      if (group.datasets.size() != info.channels.size())
        continue;

      if (m_masterTimeChannel)
        m_masterTimeChannel->SetChannelValue(timestamp);

      for (size_t i = 0; i < group.datasets.size(); ++i)
      {
        if (i < info.channels.size() && i < info.isNumeric.size())
        {
          const auto &dataset = group.datasets.at(i);
          auto *channel = info.channels[i];

          if (info.isNumeric[i])
            channel->SetChannelValue(dataset.numericValue);
          else
            channel->SetChannelValue(dataset.value.toStdString());
        }
      }

      const auto timestamp_ns = static_cast<uint64_t>(timestamp * 1000000000.0);
      m_writer->SaveSample(*info.channelGroup, timestamp_ns);
    }
  }
}

/**
 * @brief Closes the MDF4 file
 */
void MDF4::ExportWorker::closeFile()
{
  if (isOpen() && m_writer)
  {
    writeValues();

    const auto stop_time
        = static_cast<uint64_t>(QDateTime::currentMSecsSinceEpoch() * 1000000);
    m_writer->StopMeasurement(stop_time);
    m_writer->FinalizeMeasurement();

    m_fileOpen = false;
    m_writer.reset();
    m_groupMap.clear();
    m_masterTimeChannel = nullptr;

    Q_EMIT openChanged();
  }
}

/**
 * @brief Creates a new MDF4 file with hierarchical structure
 */
void MDF4::ExportWorker::createFile(const JSON::Frame &frame)
{
  if (isOpen())
    closeFile();

  if (!SerialStudio::activated())
    return;

  const auto dateTime = QDateTime::currentDateTime();
  const auto fileName = dateTime.toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss"))
                        + QStringLiteral(".mf4");

  const auto frameName
      = frame.title.isEmpty() ? QStringLiteral("SerialStudio") : frame.title;
  QDir dir(Misc::WorkspaceManager::instance().path("MDF4"));
  if (!dir.exists(frameName))
    dir.mkpath(frameName);

  dir.cd(frameName);
  m_filePath = dir.filePath(fileName);

  try
  {
    m_writer = mdf::MdfFactory::CreateMdfWriter(mdf::MdfWriterType::Mdf4Basic);
    if (!m_writer)
      return;

    m_writer->Init(m_filePath.toStdString());

    auto *header = m_writer->Header();
    if (!header)
      return;

    header->Author("Serial Studio");
    header->Description(
        "Generated by Serial Studio - https://serial-studio.com/");
    header->Subject(frameName.toStdString());
    header->Project("Telemetry Data");
    header->StartTime(dateTime.toMSecsSinceEpoch() * 1000000);

    auto *dataGroup = m_writer->CreateDataGroup();
    if (!dataGroup)
      return;

    dataGroup->Description("Serial Studio Data");

    bool firstGroup = true;
    for (const auto &group : frame.groups)
    {
      auto *channelGroup = dataGroup->CreateChannelGroup();
      if (!channelGroup)
        continue;

      channelGroup->Name(group.title.toStdString());

      ChannelGroupInfo info;
      info.channelGroup = channelGroup;

      if (firstGroup)
      {
        auto *timeChannel = channelGroup->CreateChannel();
        if (!timeChannel)
          continue;

        timeChannel->Name("Time");
        timeChannel->Unit("s");
        timeChannel->Type(mdf::ChannelType::Master);
        timeChannel->DataType(mdf::ChannelDataType::FloatLe);
        timeChannel->DataBytes(8);
        m_masterTimeChannel = timeChannel;

        firstGroup = false;
      }

      for (const auto &dataset : group.datasets)
      {
        auto *channel = channelGroup->CreateChannel();
        if (!channel)
          continue;

        channel->Name(dataset.title.toStdString());
        channel->Unit(dataset.units.toStdString());
        channel->Type(mdf::ChannelType::FixedLength);

        if (dataset.isNumeric)
        {
          channel->DataType(mdf::ChannelDataType::FloatLe);
          channel->DataBytes(8);
        }
        else
        {
          channel->DataType(mdf::ChannelDataType::StringAscii);
          channel->DataBytes(256);
        }

        info.channels.push_back(channel);
        info.isNumeric.push_back(dataset.isNumeric);
      }

      m_groupMap[group.groupId] = info;
    }

    m_writer->InitMeasurement();
    m_writer->StartMeasurement(dateTime.toMSecsSinceEpoch() * 1000000);

    m_fileOpen = true;
    Q_EMIT openChanged();
  }

  catch (const std::exception &e)
  {
    qWarning() << "MDF4 Export: Failed to create file:" << e.what();
    m_fileOpen = false;
    m_writer.reset();
  }
}

#endif

//------------------------------------------------------------------------------
// Export implementation
//------------------------------------------------------------------------------

/**
 * Constructor function, configures the export settings
 */
MDF4::Export::Export()
  : m_isOpen(false)
  , m_exportEnabled(false)
#ifdef BUILD_COMMERCIAL
  , m_worker(nullptr)
  , m_queueSize(0)
#endif
{
#ifdef BUILD_COMMERCIAL
  m_worker = new ExportWorker(&m_pendingFrames, &m_exportEnabled, &m_queueSize);
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

  setExportEnabled(m_settings.value("MDF4Export", false).toBool());
}

/**
 * Close file & finish write-operations before destroying the class.
 */
MDF4::Export::~Export()
{
#ifdef BUILD_COMMERCIAL
  m_workerThread.quit();
  m_workerThread.wait();
#endif
}

/**
 * Returns a pointer to the only instance of this class.
 */
MDF4::Export &MDF4::Export::instance()
{
  static Export instance;
  return instance;
}

/**
 * Returns true if the MDF4 output file is open.
 */
bool MDF4::Export::isOpen() const
{
#ifdef BUILD_COMMERCIAL
  return m_isOpen.load(std::memory_order_relaxed);
#else
  return false;
#endif
}

/**
 * Returns true if the MDF4 export module is enabled.
 */
bool MDF4::Export::exportEnabled() const
{
#ifdef BUILD_COMMERCIAL
  return m_exportEnabled.load(std::memory_order_relaxed);
#else
  return false;
#endif
}

/**
 * Write all remaining data & close the output file.
 */
void MDF4::Export::closeFile()
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
void MDF4::Export::setupExternalConnections()
{
#ifdef BUILD_COMMERCIAL
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this,
          &Export::closeFile);
  connect(&IO::Manager::instance(), &IO::Manager::pausedChanged, this, [this] {
    if (IO::Manager::instance().paused())
      closeFile();
  });
#endif
}

/**
 * Enables or disables data export.
 */
void MDF4::Export::setExportEnabled(const bool enabled)
{
#ifdef BUILD_COMMERCIAL
  if (SerialStudio::activated())
  {
    m_exportEnabled.store(enabled, std::memory_order_relaxed);
    Q_EMIT enabledChanged();

    if (!enabled && isOpen())
      closeFile();

    m_settings.setValue("MDF4Export", enabled);
    return;
  }
#endif

  closeFile();
  m_exportEnabled.store(false, std::memory_order_relaxed);
  m_settings.setValue("MDF4Export", false);
  Q_EMIT enabledChanged();

  if (enabled)
    Misc::Utilities::showMessageBox(
        tr("MDF4 Export is a Pro feature."),
        tr("This feature requires a license. Please purchase one to enable "
           "MDF4 export."));
}

/**
 * Receives frame data and enqueues it for export
 */
void MDF4::Export::hotpathTxFrame(const JSON::Frame &frame)
{
#ifdef BUILD_COMMERCIAL
  if (!exportEnabled() || MDF4::Player::instance().isOpen())
    return;

  if (!IO::Manager::instance().isConnected())
    return;

  if (m_queueSize.load(std::memory_order_relaxed) >= kMDF4ExportQueueCapacity)
  {
    qWarning() << "MDF4 Export: Dropping frame (queue full)";
    return;
  }

  if (!m_pendingFrames.enqueue(TimestampFrame(JSON::Frame(frame))))
  {
    qWarning() << "MDF4 Export: Failed to enqueue frame";
    return;
  }

  const auto size = m_queueSize.fetch_add(1, std::memory_order_relaxed) + 1;
  if (size >= kMDF4FlushThreshold)
    QMetaObject::invokeMethod(m_worker, &ExportWorker::writeValues,
                              Qt::QueuedConnection);
#else
  (void)frame;
#endif
}

#ifdef BUILD_COMMERCIAL
/**
 * Called when the worker's file open state changes
 */
void MDF4::Export::onWorkerOpenChanged()
{
  m_isOpen.store(m_worker->isOpen(), std::memory_order_relaxed);
  Q_EMIT openChanged();
}
#endif

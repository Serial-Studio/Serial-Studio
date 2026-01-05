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
#  include "CSV/Player.h"
#  include "MDF4/Player.h"
#  include "MQTT/Client.h"
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
 * @brief Destructor for the MDF4 export worker
 */
MDF4::ExportWorker::~ExportWorker() = default;

/**
 * @brief Returns true if file is open
 */
bool MDF4::ExportWorker::isResourceOpen() const
{
  return m_fileOpen;
}

/**
 * @brief Processes a batch of MDF4 frames
 *
 * Writes frames to MDF4 file using the mdflib writer.
 * If no file is open, a new file is created before writing.
 *
 * @param items Vector of timestamped frames to process.
 */
void MDF4::ExportWorker::processItems(
    const std::vector<DataModel::TimestampedFramePtr> &items)
{
  if (items.empty())
    return;

  if (!IO::Manager::instance().isConnected())
    return;

  if (!isResourceOpen() && !items.empty())
  {
    createFile(items.front()->data);
    m_steadyBaseline = items.front()->timestamp;
    m_systemBaseline = std::chrono::system_clock::now();
  }

  if (!isResourceOpen() || !m_writer)
    return;

  for (const auto &frame : items)
  {
    const auto steadyOffset = frame->timestamp - m_steadyBaseline;
    const auto systemTime = m_systemBaseline + steadyOffset;
    const auto timestamp_ns
        = std::chrono::duration_cast<std::chrono::nanoseconds>(
              systemTime.time_since_epoch())
              .count();
    const double timestamp_s
        = static_cast<double>(timestamp_ns) / 1'000'000'000.0;

    for (const auto &group : frame->data.groups)
    {
      auto it = m_groupMap.find(group.groupId);
      if (it == m_groupMap.end())
        continue;

      auto &info = it->second;
      if (group.datasets.size() != info.channels.size())
        continue;

      if (m_masterTimeChannel)
        m_masterTimeChannel->SetChannelValue(timestamp_s);

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

      m_writer->SaveSample(*info.channelGroup,
                           static_cast<uint64_t>(timestamp_ns));
    }
  }
}

/**
 * @brief Closes the MDF4 file
 */
void MDF4::ExportWorker::closeResources()
{
  if (isResourceOpen() && m_writer)
  {
    const auto steadyNow = DataModel::TimestampedFrame::SteadyClock::now();
    const auto steadyOffset = steadyNow - m_steadyBaseline;
    const auto systemTime = m_systemBaseline + steadyOffset;
    const auto stop_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
                               systemTime.time_since_epoch())
                               .count();

    m_writer->StopMeasurement(static_cast<uint64_t>(stop_time));
    m_writer->FinalizeMeasurement();

    m_fileOpen = false;
    m_writer.reset();
    m_groupMap.clear();
    m_masterTimeChannel = nullptr;
  }
}

/**
 * @brief Creates a new MDF4 file with hierarchical structure
 */
void MDF4::ExportWorker::createFile(const DataModel::Frame &frame)
{
  if (isResourceOpen())
    closeResources();

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
    Q_EMIT resourceOpenChanged();
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
#ifdef BUILD_COMMERCIAL
  : DataModel::FrameConsumer<DataModel::TimestampedFramePtr>(
        {.queueCapacity = 8192,
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

  setExportEnabled(m_settings.value("MDF4Export", false).toBool());
}

/**
 * Close file & finish write-operations before destroying the class.
 */
MDF4::Export::~Export() = default;

#ifdef BUILD_COMMERCIAL
/**
 * @brief Creates the MDF4 export worker instance.
 *
 * @return Pointer to newly created ExportWorker.
 */
DataModel::FrameConsumerWorkerBase *MDF4::Export::createWorker()
{
  return new ExportWorker(&m_pendingQueue, &m_consumerEnabled, &m_queueSize);
}
#endif

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
  return consumerEnabled();
#else
  return false;
#endif
}

/**
 * Write all remaining data & close the output file.
 *
 * Flushes any remaining data in the queue before closing to prevent
 * creating small trailing files.
 */
void MDF4::Export::closeFile()
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
    if (!enabled && isOpen())
      closeFile();

    setConsumerEnabled(enabled);
    m_settings.setValue("MDF4Export", enabled);
    Q_EMIT enabledChanged();
    return;
  }

  closeFile();
  setConsumerEnabled(false);
  m_settings.setValue("MDF4Export", false);
  Q_EMIT enabledChanged();
#else
  closeFile();
  m_exportEnabled.store(false, std::memory_order_relaxed);
  m_settings.setValue("MDF4Export", false);
  Q_EMIT enabledChanged();
#endif

  if (enabled)
    Misc::Utilities::showMessageBox(
        tr("MDF4 Export is a Pro feature."),
        tr("This feature requires a license. Please purchase one to enable "
           "MDF4 export."));
}

/**
 * Receives timestamped frame data and enqueues it for export
 */
void MDF4::Export::hotpathTxFrame(const DataModel::TimestampedFramePtr &frame)
{
#ifdef BUILD_COMMERCIAL
  if (!exportEnabled() || SerialStudio::isAnyPlayerOpen())
    return;

  enqueueData(frame);
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
  auto *worker = static_cast<ExportWorker *>(m_worker);
  m_isOpen.store(worker->isResourceOpen(), std::memory_order_relaxed);
  Q_EMIT openChanged();
}
#endif

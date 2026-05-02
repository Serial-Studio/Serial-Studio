/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <QDateTime>
#include <QDir>
#include <QTimer>

#include "AppState.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

#ifdef BUILD_COMMERCIAL
#  include <mdf/ichannel.h>
#  include <mdf/ichannelgroup.h>
#  include <mdf/idatagroup.h>
#  include <mdf/iheader.h>
#  include <mdf/mdffactory.h>
#  include <mdf/mdfwriter.h>

#  include "CSV/Player.h"
#  include "DataModel/FrameBuilder.h"
#  include "IO/ConnectionManager.h"
#  include "Licensing/CommercialToken.h"
#  include "Licensing/LemonSqueezy.h"
#  include "MDF4/Player.h"
#  include "Misc/WorkspaceManager.h"
#  include "MQTT/Client.h"
#endif

//--------------------------------------------------------------------------------------------------
// ExportWorker implementation
//--------------------------------------------------------------------------------------------------

#ifdef BUILD_COMMERCIAL

/**
 * @brief Constructor for the MDF4 export worker
 */
MDF4::ExportWorker::ExportWorker(
  moodycamel::ReaderWriterQueue<DataModel::TimestampedFramePtr>* queue,
  std::atomic<bool>* enabled,
  std::atomic<size_t>* queueSize)
  : FrameConsumerWorker(queue, enabled, queueSize), m_fileOpen(false)
{}

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
 * @brief Writes all dataset values (final + raw) for one channel group.
 */
void MDF4::ExportWorker::writeGroupDatasets(const DataModel::Group& group, ChannelGroupInfo& info)
{
  for (size_t i = 0; i < group.datasets.size(); ++i) {
    const auto& dataset = group.datasets.at(i);

    // Write final (post-transform) values
    auto* channel = info.channels[i];
    if (info.isNumeric[i])
      channel->SetChannelValue(dataset.numericValue);
    else
      channel->SetChannelValue(dataset.value.toStdString());

    // Skip raw channel when absent
    if (i >= info.rawChannels.size() || !info.rawChannels[i])
      continue;

    auto* rawCh = info.rawChannels[i];
    if (info.isNumeric[i])
      rawCh->SetChannelValue(dataset.rawNumericValue);
    else
      rawCh->SetChannelValue(dataset.rawValue.toStdString());
  }
}

/**
 * @brief Persists one frame's groups to the MDF4 writer at the given timestamp.
 */
void MDF4::ExportWorker::writeFrameGroups(const DataModel::Frame& frame,
                                          qint64 timestamp_ns,
                                          double timestamp_s)
{
  for (const auto& group : frame.groups) {
    auto it = m_groupMap.find(group.groupId);
    if (it == m_groupMap.end())
      continue;

    auto& info = it->second;
    if (group.datasets.size() != info.channels.size())
      continue;

    if (info.timeChannel)
      info.timeChannel->SetChannelValue(timestamp_s);

    writeGroupDatasets(group, info);
    m_writer->SaveSample(*info.channelGroup, static_cast<uint64_t>(timestamp_ns));
  }
}

/**
 * @brief Processes a batch of MDF4 frames
 *
 * Writes frames to MDF4 file using the mdflib writer.
 * If no file is open, a new file is created before writing.
 *
 * @param items Vector of timestamped frames to process.
 */
void MDF4::ExportWorker::processItems(const std::vector<DataModel::TimestampedFramePtr>& items)
{
  // Skip empty batches or when disconnected
  if (items.empty())
    return;

  if (!IO::ConnectionManager::instance().isConnected())
    return;

  if (!isResourceOpen()) {
    createFile(items.front()->data);
    m_steadyBaseline = items.front()->timestamp;
    m_systemBaseline = std::chrono::system_clock::now();
    resetMonotonicClock();
  }

  if (!isResourceOpen() || !m_writer)
    return;

  // Guard mdflib calls against exceptions propagating through Qt's event loop
  try {
    const auto systemEpochNs =
      std::chrono::duration_cast<std::chrono::nanoseconds>(m_systemBaseline.time_since_epoch())
        .count();

    for (const auto& frame : items) {
      // Monotonic offset from session start, shifted onto system-clock epoch
      const qint64 offsetNs     = monotonicFrameNs(frame->timestamp, m_steadyBaseline);
      const qint64 timestamp_ns = systemEpochNs + offsetNs;
      const double timestamp_s  = static_cast<double>(timestamp_ns) / 1'000'000'000.0;

      writeFrameGroups(frame->data, timestamp_ns, timestamp_s);
    }
  } catch (const std::exception& e) {
    qWarning() << "[MDF4] Exception in processItems:" << e.what();
  }
}

/**
 * @brief Closes the MDF4 file
 */
void MDF4::ExportWorker::closeResources()
{
  if (isResourceOpen() && m_writer) {
    // Finalize the MDF4 measurement
    try {
      const auto steadyNow    = DataModel::TimestampedFrame::SteadyClock::now();
      const auto steadyOffset = steadyNow - m_steadyBaseline;
      const auto systemTime   = m_systemBaseline + steadyOffset;
      const auto stop_time =
        std::chrono::duration_cast<std::chrono::nanoseconds>(systemTime.time_since_epoch()).count();

      m_writer->StopMeasurement(static_cast<uint64_t>(stop_time));
      m_writer->FinalizeMeasurement();
    } catch (const std::exception& e) {
      qWarning() << "[MDF4] Exception in closeResources:" << e.what();
    }

    // Release writer and clear state
    m_fileOpen = false;
    m_writer.reset();
    m_groupMap.clear();
    DataModel::clear_frame(m_templateFrame);
  }
}

/**
 * @brief Sanitizes a frame title for use as a directory name.
 */
static QString sanitizeFrameTitle(const QString& title)
{
  QString frameName = title;
  frameName.remove(QChar('/'));
  frameName.remove(QChar('\\'));
  frameName.remove(QStringLiteral(".."));
  if (frameName.isEmpty())
    frameName = QStringLiteral("SerialStudio");

  return frameName;
}

/**
 * @brief Configures an mdflib data channel as either numeric (FloatLe/8) or string (Ascii/256).
 */
static void configureChannelType(mdf::IChannel* channel, bool isNum)
{
  channel->Type(mdf::ChannelType::FixedLength);
  if (isNum) {
    channel->DataType(mdf::ChannelDataType::FloatLe);
    channel->DataBytes(8);
    return;
  }

  channel->DataType(mdf::ChannelDataType::StringAscii);
  channel->DataBytes(256);
}

/**
 * @brief Creates a configured master time channel on the given channel group.
 */
static mdf::IChannel* createTimeChannel(mdf::IChannelGroup* channelGroup)
{
  auto* timeChannel = channelGroup->CreateChannel();
  if (!timeChannel)
    return nullptr;

  timeChannel->Name("Time");
  timeChannel->Unit("s");
  timeChannel->Type(mdf::ChannelType::Master);
  timeChannel->DataType(mdf::ChannelDataType::FloatLe);
  timeChannel->DataBytes(8);
  return timeChannel;
}

/**
 * @brief Builds a (groupId, datasetId) -> isNumeric lookup from a live frame.
 */
static std::map<std::pair<int, int>, bool> buildNumericLookup(const DataModel::Frame& frame)
{
  std::map<std::pair<int, int>, bool> numericLookup;
  for (const auto& g : frame.groups)
    for (const auto& d : g.datasets)
      numericLookup[{g.groupId, d.datasetId}] = d.isNumeric;

  return numericLookup;
}

/**
 * @brief Appends final + raw channels for one dataset to the supplied info struct.
 */
void MDF4::ExportWorker::addDatasetChannels(mdf::IChannelGroup* channelGroup,
                                            const DataModel::Dataset& dataset,
                                            bool isNum,
                                            ChannelGroupInfo& info)
{
  // Final (post-transform) channel
  auto* channel = channelGroup->CreateChannel();
  if (!channel)
    return;

  channel->Name(dataset.title.toStdString());
  channel->Unit(dataset.units.toStdString());
  configureChannelType(channel, isNum);

  info.channels.push_back(channel);
  info.isNumeric.push_back(isNum);

  // Raw (pre-transform) channel -- always pushed (null included) to keep indices aligned
  auto* rawChannel = channelGroup->CreateChannel();
  if (rawChannel) {
    rawChannel->Name(dataset.title.toStdString() + " (raw)");
    rawChannel->Unit(dataset.units.toStdString());
    configureChannelType(rawChannel, isNum);
  }

  info.rawChannels.push_back(rawChannel);
}

/**
 * @brief Builds and registers the channel group for one project group.
 */
void MDF4::ExportWorker::buildChannelGroupForGroup(
  mdf::IDataGroup* dataGroup,
  const DataModel::Group& group,
  const QString& sourceTitle,
  bool usingTemplate,
  const std::map<std::pair<int, int>, bool>& numericLookup)
{
  auto* channelGroup = dataGroup->CreateChannelGroup();
  if (!channelGroup)
    return;

  const auto cgName =
    sourceTitle.isEmpty() ? group.title : QStringLiteral("%1 / %2").arg(sourceTitle, group.title);
  channelGroup->Name(cgName.toStdString());

  ChannelGroupInfo info;
  info.channelGroup = channelGroup;
  info.timeChannel  = createTimeChannel(channelGroup);

  for (const auto& dataset : group.datasets) {
    // Resolve data type from live frame when using template
    bool isNum = dataset.isNumeric;
    if (usingTemplate) {
      auto nit = numericLookup.find({group.groupId, dataset.datasetId});
      isNum    = (nit != numericLookup.end()) ? nit->second : true;
    }

    addDatasetChannels(channelGroup, dataset, isNum, info);
  }

  m_groupMap[group.groupId] = info;
}

/**
 * @brief Builds the MDF4 channel groups from the project (or live) frame definition.
 */
void MDF4::ExportWorker::buildChannelGroups(mdf::IDataGroup* dataGroup,
                                            const DataModel::Frame& frame)
{
  // Prefer cached project frame, fall back to first data frame
  const bool usingTemplate = !m_templateFrame.groups.empty();
  const auto& allGroups    = usingTemplate ? m_templateFrame.groups : frame.groups;

  QMap<int, QString> sourceTitles;
  const auto& srcRefs = usingTemplate ? m_templateFrame.sources : frame.sources;
  for (const auto& s : srcRefs)
    sourceTitles.insert(s.sourceId, s.title);

  std::map<std::pair<int, int>, bool> numericLookup;
  if (usingTemplate)
    numericLookup = buildNumericLookup(frame);

  // Skip image groups entirely -- they have no telemetry datasets
  for (const auto& group : allGroups) {
    if (group.widget == QLatin1String("image"))
      continue;

    buildChannelGroupForGroup(
      dataGroup, group, sourceTitles.value(group.sourceId), usingTemplate, numericLookup);
  }
}

/**
 * @brief Initializes the writer + header structures for a new file.
 */
bool MDF4::ExportWorker::initWriterAndHeader(const QString& frameName, const QDateTime& dateTime)
{
  m_writer = mdf::MdfFactory::CreateMdfWriter(mdf::MdfWriterType::Mdf4Basic);
  if (!m_writer)
    return false;

  m_writer->Init(m_filePath.toStdString());

  auto* header = m_writer->Header();
  if (!header)
    return false;

  header->Author("Serial Studio");
  header->Description("Generated by Serial Studio - https://serial-studio.com/");
  header->Subject(frameName.toStdString());
  header->Project("Telemetry Data");
  header->StartTime(dateTime.toMSecsSinceEpoch() * 1000000);
  return true;
}

/**
 * @brief Creates a new MDF4 file with hierarchical structure
 */
void MDF4::ExportWorker::createFile(const DataModel::Frame& frame)
{
  // Close any previously open file
  if (isResourceOpen())
    closeResources();

  // Validate license before creating the file
  const auto& token = Licensing::CommercialToken::current();
  if (!token.isValid() || !SS_LICENSE_GUARD() || token.featureTier() < Licensing::FeatureTier::Pro)
    return;

  const auto dateTime = QDateTime::currentDateTime();
  const auto fileName =
    dateTime.toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss")) + QStringLiteral(".mf4");
  const QString frameName = sanitizeFrameTitle(frame.title);

  QDir dir(Misc::WorkspaceManager::instance().path("MDF4"));
  if (!dir.exists(frameName))
    dir.mkpath(frameName);

  dir.cd(frameName);
  m_filePath = dir.filePath(fileName);

  try {
    if (!initWriterAndHeader(frameName, dateTime))
      return;

    auto* dataGroup = m_writer->CreateDataGroup();
    if (!dataGroup)
      return;

    dataGroup->Description("Serial Studio Data");
    buildChannelGroups(dataGroup, frame);

    m_writer->InitMeasurement();
    m_writer->StartMeasurement(dateTime.toMSecsSinceEpoch() * 1000000);

    m_fileOpen = true;
    Q_EMIT resourceOpenChanged();
  } catch (const std::exception& e) {
    qWarning() << "[MDF4] Failed to create file:" << e.what();
    m_fileOpen = false;
    m_writer.reset();
  } catch (...) {
    qWarning() << "[MDF4] Failed to create file: unknown exception";
    m_fileOpen = false;
    m_writer.reset();
  }
}

#endif

//--------------------------------------------------------------------------------------------------
// Export implementation
//--------------------------------------------------------------------------------------------------

/**
 * Constructor function, configures the export settings
 */
MDF4::Export::Export()
#ifdef BUILD_COMMERCIAL
  : DataModel::FrameConsumer<DataModel::TimestampedFramePtr>(
      {.queueCapacity = 8192, .flushThreshold = 1024, .timerIntervalMs = 1000})
  , m_isOpen(false)
  , m_exportEnabled(false)
  , m_persistSettings(true)
#else
  : m_isOpen(false), m_exportEnabled(false), m_persistSettings(true)
#endif
{
#ifdef BUILD_COMMERCIAL
  // Wire worker file-state and license-revocation signals
  initializeWorker();
  connect(m_worker,
          &ExportWorker::resourceOpenChanged,
          this,
          &Export::onWorkerOpenChanged,
          Qt::QueuedConnection);

  connect(&Licensing::LemonSqueezy::instance(),
          &Licensing::LemonSqueezy::activatedChanged,
          this,
          [=, this] {
            if (exportEnabled()
                && (!Licensing::CommercialToken::current().isValid() || !SS_LICENSE_GUARD()))
              setExportEnabled(false);
          });
#endif

  // Restore persisted export preference
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
DataModel::FrameConsumerWorkerBase* MDF4::Export::createWorker()
{
  return new ExportWorker(&m_pendingQueue, &m_consumerEnabled, &m_queueSize);
}
#endif

/**
 * Returns a pointer to the only instance of this class.
 */
MDF4::Export& MDF4::Export::instance()
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
  auto* worker = static_cast<ExportWorker*>(m_worker);
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
  connect(
    &IO::ConnectionManager::instance(), &IO::ConnectionManager::connectedChanged, this, [this] {
      if (IO::ConnectionManager::instance().isConnected()) {
        // Cache project frame for file creation (only valid in ProjectFile mode)
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
#endif
}

/**
 * @brief Toggles whether export-enabled changes get written to QSettings.
 *
 * Set false at startup for shortcut/runtime-mode launches so that toggling
 * export from a shortcut never bleeds back into the user's main app settings.
 */
void MDF4::Export::setSettingsPersistent(const bool persistent)
{
  m_persistSettings = persistent;
}

/**
 * Enables or disables data export.
 */
void MDF4::Export::setExportEnabled(const bool enabled)
{
  // Refuse to enable while the app is in Console-only mode
  const bool allow = enabled && AppState::instance().operationMode() != SerialStudio::ConsoleOnly;

  // Validate license and apply the export state
#ifdef BUILD_COMMERCIAL
  const auto& tk = Licensing::CommercialToken::current();
  if (tk.isValid() && SS_LICENSE_GUARD() && tk.featureTier() >= Licensing::FeatureTier::Pro) {
    if (!allow && isOpen())
      closeFile();

    setConsumerEnabled(allow);
    if (m_persistSettings)
      m_settings.setValue("MDF4Export", allow);

    Q_EMIT enabledChanged();
    return;
  }

  // License invalid or missing -- force disable
  closeFile();
  setConsumerEnabled(false);
  if (m_persistSettings)
    m_settings.setValue("MDF4Export", false);

  Q_EMIT enabledChanged();
#else
  closeFile();
  m_exportEnabled.store(false, std::memory_order_relaxed);
  if (m_persistSettings)
    m_settings.setValue("MDF4Export", false);

  Q_EMIT enabledChanged();
#endif

  if (enabled)
    Misc::Utilities::showMessageBox(
      tr("MDF4 Export is a Pro feature."),
      tr("This feature requires a license. Please purchase one to enable MDF4 export."));
}

/**
 * Receives timestamped frame data and enqueues it for export
 */
void MDF4::Export::hotpathTxFrame(const DataModel::TimestampedFramePtr& frame)
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
  auto* worker = static_cast<ExportWorker*>(m_worker);
  m_isOpen.store(worker->isResourceOpen(), std::memory_order_relaxed);
  Q_EMIT openChanged();
}
#endif

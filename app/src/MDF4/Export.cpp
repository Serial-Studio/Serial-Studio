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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "Export.h"

#include <QDateTime>
#include <QDir>
#include <QTimer>

#include "AppState.h"
#include "IO/PipelineHost.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"
#include "SSAssert.h"

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
#endif

//--------------------------------------------------------------------------------------------------
// ExportWorker implementation
//--------------------------------------------------------------------------------------------------

#ifdef BUILD_COMMERCIAL

/**
 * @brief Constructor for the MDF4 export worker
 */
MDF4::ExportWorker::ExportWorker(moodycamel::ReaderWriterQueue<DataModel::DataBlockPtr>* queue,
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
 * @brief Resolves every dataset's uniqueId to the channel group and slot its samples land in, so a
 *        block column can address its channels directly. Built once at file creation; the block
 *        carries no group structure, only dataset identities.
 */
void MDF4::ExportWorker::buildColumnMap(const DataModel::Frame& frame)
{
  m_columnMap.clear();
  for (const auto& group : frame.groups) {
    const auto it = m_groupMap.find(group.groupId);
    if (it == m_groupMap.end())
      continue;

    for (std::size_t i = 0; i < group.datasets.size(); ++i) {
      if (i >= it->second.channels.size())
        break;

      m_columnMap[group.datasets[i].uniqueId] = ColumnSlot{group.groupId, i};
    }
  }
}

/**
 * @brief Resolves each of @p block's columns to its channels once, so the per-sample write is an
 *        indexed load rather than two std::map lookups and a linear scan repeated per sample.
 */
void MDF4::ExportWorker::resolveBlockColumns(const DataModel::DataBlock& block)
{
  m_resolved.assign(block.columns.size(), ResolvedColumn{});
  m_touchedGroups.clear();

  for (std::size_t c = 0; c < block.columns.size(); ++c) {
    const auto mapped = m_columnMap.find(block.columns[c].uniqueId);
    if (mapped == m_columnMap.end())
      continue;

    auto group = m_groupMap.find(mapped->second.groupId);
    if (group == m_groupMap.end())
      continue;

    auto& info    = group->second;
    const auto ch = mapped->second.index;
    auto& out     = m_resolved[c];

    out.valid      = true;
    out.groupId    = mapped->second.groupId;
    out.isNumeric  = ch < info.isNumeric.size() ? info.isNumeric[ch] : true;
    out.channel    = ch < info.channels.size() ? info.channels[ch] : nullptr;
    out.rawChannel = ch < info.rawChannels.size() ? info.rawChannels[ch] : nullptr;

    if (std::find(m_touchedGroups.begin(), m_touchedGroups.end(), out.groupId)
        == m_touchedGroups.end())
      m_touchedGroups.push_back(out.groupId);
  }
}

/**
 * @brief Writes sample @p index of @p block into its channels and saves every channel group the
 *        block touched. Each group keeps its own master time channel, which is what lets one file
 *        hold sources at different rates (spec 0055 R5), so the same-instant tie-break is per
 *        source too -- a worker-wide one rewrote a second source's masters into a staircase (B1).
 */
void MDF4::ExportWorker::writeBlockSample(const DataModel::DataBlock& block,
                                          qsizetype index,
                                          qint64 systemEpochNs)
{
  const auto slot = static_cast<std::size_t>(index);
  SS_ASSERT(m_resolved.size() == block.columns.size(), return);

  for (std::size_t c = 0; c < block.columns.size(); ++c) {
    const auto& target = m_resolved[c];
    if (!target.valid)
      continue;

    const auto& column = block.columns[c];
    const bool isNum   = target.isNumeric;

    if (target.channel) {
      if (isNum || !column.hasText)
        target.channel->SetChannelValue(column.values[slot]);
      else
        target.channel->SetChannelValue(column.text[slot].toStdString());
    }

    if (column.hasRaw && target.rawChannel) {
      if (isNum || !column.hasText)
        target.rawChannel->SetChannelValue(column.rawValues[slot]);
      else
        target.rawChannel->SetChannelValue(column.rawText[slot].toStdString());
    }
  }

  const auto stamp = DataModel::sample_time(block, index);
  qint64 offsetNs =
    std::chrono::duration_cast<std::chrono::nanoseconds>(stamp - m_steadyBaseline).count();
  if (!DataModel::uniform_grid(block))
    offsetNs = monotonicSourceNs(block.sourceId, offsetNs);

  const qint64 timestampNs = systemEpochNs + offsetNs;

  // code-verify off
  // The master is written by SaveSample from the timestamp argument. Assigning absolute epoch
  // seconds to the time channel first was dead and misleading: mdflib overwrote it with the
  // group's own relative time (B10).
  // code-verify on
  for (const int groupId : m_touchedGroups) {
    auto group = m_groupMap.find(groupId);
    if (group == m_groupMap.end())
      continue;

    m_writer->SaveSample(*group->second.channelGroup, static_cast<uint64_t>(timestampNs));
  }
}

/**
 * @brief Processes a batch of blocks. Connectivity only gates the creation of a NEW file: an
 *        already-open writer still receives the queued backlog, so the close() drain after a
 *        disconnect flushes captured samples instead of silently dropping them.
 */
void MDF4::ExportWorker::processItems(const std::vector<DataModel::DataBlockPtr>& items)
{
  if (items.empty())
    return;

  if (!isResourceOpen()) {
    static auto& pipeline = IO::PipelineHost::instance();
    if (!pipeline.pipelineConnected())
      return;

    if (!m_structure.hasStructure())
      return;

    createFile(m_structure.templateFrame());
    buildColumnMap(m_structure.templateFrame());
    m_steadyBaseline = items.front()->t0;
    m_systemBaseline = std::chrono::system_clock::now();
    resetMonotonicClock();
  }

  if (!isResourceOpen() || !m_writer)
    return;

  try {
    const auto systemEpochNs =
      std::chrono::duration_cast<std::chrono::nanoseconds>(m_systemBaseline.time_since_epoch())
        .count();

    for (const auto& block : items) {
      if (!block || block->samples <= 0)
        continue;

      resolveBlockColumns(*block);
      for (qsizetype i = 0; i < block->samples; ++i)
        writeBlockSample(*block, i, systemEpochNs);
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

    m_fileOpen = false;
    m_writer.reset();
    m_groupMap.clear();
    m_columnMap.clear();
    m_structure.clear();
  }
}

/**
 * @brief Stores the schema template frame; must run on the worker thread (queued invoke) so the
 *        assignment never races processItems() or closeResources().
 */
void MDF4::ExportWorker::setTemplateFrame(const DataModel::Frame& frame)
{
  m_structure.setTemplateFrame(frame);
}

/**
 * @brief Adopts the structure the pipeline publishes when the connect-time fetch came back empty
 *        (QuickPlot derives its datasets from the first frame, so at connect there is none).
 */
void MDF4::ExportWorker::applyPublishedStructure(const DataModel::Frame& frame)
{
  m_structure.applyPublishedStructure(frame, isResourceOpen());
}

/**
 * @brief Configures an mdflib data channel as either numeric (FloatLe/8) or string (UTF-8/256).
 *        The strings fed to it are toStdString() of a QString, i.e. UTF-8, so declaring ISO-8859-1
 *        mislabelled every non-ASCII value in the file (B16).
 */
static void configureChannelType(mdf::IChannel* channel, bool isNum)
{
  channel->Type(mdf::ChannelType::FixedLength);
  if (isNum) {
    channel->DataType(mdf::ChannelDataType::FloatLe);
    channel->DataBytes(8);
    return;
  }

  channel->DataType(mdf::ChannelDataType::StringUTF8);
  channel->DataBytes(256);
}

/**
 * @brief Creates a configured master time channel on the given channel group. ASAM MDF 4.1 requires
 *        a master to declare its sync type (1..4); leaving it at 0 made the file non-conforming
 *        and readable only through mdflib's MDF3 fallback (B10).
 */
static mdf::IChannel* createTimeChannel(mdf::IChannelGroup* channelGroup)
{
  auto* timeChannel = channelGroup->CreateChannel();
  if (!timeChannel)
    return nullptr;

  timeChannel->Name("Time");
  timeChannel->Unit("s");
  timeChannel->Type(mdf::ChannelType::Master);
  timeChannel->Sync(mdf::ChannelSyncType::Time);
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
  auto* channel = channelGroup->CreateChannel();
  if (!channel)
    return;

  channel->Name(dataset.title.toStdString());
  channel->Unit(dataset.units.toStdString());
  configureChannelType(channel, isNum);

  info.channels.push_back(channel);
  info.isNumeric.push_back(isNum);

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
  const bool usingTemplate = m_structure.hasStructure();
  const auto& allGroups    = usingTemplate ? m_structure.templateFrame().groups : frame.groups;

  QMap<int, QString> sourceTitles;
  const auto& srcRefs = usingTemplate ? m_structure.templateFrame().sources : frame.sources;
  for (const auto& s : srcRefs)
    sourceTitles.insert(s.sourceId, s.title);

  std::map<std::pair<int, int>, bool> numericLookup;
  if (usingTemplate)
    numericLookup = buildNumericLookup(frame);

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
  if (isResourceOpen())
    closeResources();

  const auto& token = Licensing::CommercialToken::current();
  if (!token.isValid() || !SS_LICENSE_GUARD())
    return;

  const auto dateTime = QDateTime::currentDateTime();
  const auto fileName =
    dateTime.toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss")) + QStringLiteral(".mf4");
  const QString frameName =
    DataModel::ExportStructure::sanitizeTitle(frame.title, QStringLiteral("SerialStudio"));

  const QDir dir = DataModel::ExportStructure::sessionDir(
    QStringLiteral("MDF4"), frame.title, QStringLiteral("SerialStudio"));
  if (!dir.exists())
    return;

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
  : DataModel::FrameConsumer<DataModel::DataBlockPtr>(
      {.queueCapacity = 8192, .flushThreshold = 1024, .timerIntervalMs = 1000})
  , m_isOpen(false)
  , m_exportEnabled(false)
  , m_persistSettings(true)
#else
  : m_isOpen(false), m_exportEnabled(false), m_persistSettings(true)
#endif
{
#ifdef BUILD_COMMERCIAL
  initializeWorker();
  connect(m_worker,
          &ExportWorker::resourceOpenChanged,
          this,
          &Export::onWorkerOpenChanged,
          Qt::QueuedConnection);

  static auto& lemonSqueezy = Licensing::LemonSqueezy::instance();
  connect(&lemonSqueezy, &Licensing::LemonSqueezy::activatedChanged, this, [=, this] {
    if (exportEnabled()
        && (!Licensing::CommercialToken::current().isValid() || !SS_LICENSE_GUARD()))
      setExportEnabled(false);
  });
#endif

  setExportEnabled(m_settings.value("MDF4Export", false).toBool());
}

/**
 * @brief Closes the file and finishes write operations before destroying the class.
 */
MDF4::Export::~Export() = default;

#ifdef BUILD_COMMERCIAL
/**
 * @brief Creates the MDF4 export worker instance.
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
 * @brief Returns true if the MDF4 output file is open.
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
 * @brief Returns true if the MDF4 export module is enabled.
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
 * @brief Write all remaining data & close the output file.
 */
void MDF4::Export::closeFile()
{
#ifdef BUILD_COMMERCIAL
  auto* worker = static_cast<ExportWorker*>(m_worker);
  QMetaObject::invokeMethod(worker, "close", Qt::QueuedConnection);
#endif
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Captures the current project schema frame and queues it to the worker as the template.
 */
void MDF4::Export::refreshTemplateFrame()
{
  auto* worker = static_cast<ExportWorker*>(m_worker);
  SS_ASSERT(worker != nullptr, return);

  QMetaObject::invokeMethod(
    worker,
    [worker, frame = m_sessionStructure] { worker->setTemplateFrame(frame); },
    Qt::QueuedConnection);
}
#endif

/**
 * @brief Configures signal/slot connections with the modules this exporter depends on.
 */
void MDF4::Export::setupExternalConnections()
{
#ifdef BUILD_COMMERCIAL
  connect(
    &DataModel::FrameBuilder::instance(),
    &DataModel::FrameBuilder::structurePublished,
    this,
    [this](int, const DataModel::Frame& frame) {
      auto* worker = static_cast<ExportWorker*>(m_worker);
      SS_ASSERT(worker != nullptr, return);

      QMetaObject::invokeMethod(
        worker, [worker, frame] { worker->applyPublishedStructure(frame); }, Qt::QueuedConnection);
    });
  connect(&DataModel::FrameBuilder::instance(),
          &DataModel::FrameBuilder::sessionStructureReady,
          this,
          [this](const DataModel::Frame& frame) {
            m_sessionStructure = frame;
            refreshTemplateFrame();
          });
  // code-verify off
  // Closing on the builder's session boundary rather than on connectedChanged/pausedChanged is
  // what keeps the last display tick (A2): the builder flushes its open blocks into this sink's
  // queue before emitting, and close() drains that queue before closing the file. closeResources()
  // clears the worker's template, so a boundary that leaves the link live re-pushes it.
  // code-verify on
  connect(&DataModel::FrameBuilder::instance(),
          &DataModel::FrameBuilder::sessionBoundary,
          this,
          [this](bool connected, bool paused) {
            if (!connected || paused) {
              closeFile();
              return;
            }

            refreshTemplateFrame();
          });

  connect(&AppState::instance(), &AppState::operationModeChanged, this, [this] {
    if (AppState::instance().operationMode() == SerialStudio::ConsoleOnly && exportEnabled())
      setExportEnabled(false);
  });
#endif
}

/**
 * @brief Toggles whether export-enabled changes get written to QSettings.
 */
void MDF4::Export::setSettingsPersistent(const bool persistent)
{
  m_persistSettings = persistent;
}

/**
 * @brief Enables or disables data export.
 */
void MDF4::Export::setExportEnabled(const bool enabled)
{
#ifdef BUILD_COMMERCIAL
  static auto& appState = AppState::instance();
  const auto& tk        = Licensing::CommercialToken::current();
  const bool allow      = enabled && appState.operationMode() != SerialStudio::ConsoleOnly;

  if (tk.isValid() && SS_LICENSE_GUARD()) {
    if (!allow && isOpen())
      closeFile();

    setConsumerEnabled(allow);

    if (m_persistSettings)
      m_settings.setValue("MDF4Export", allow);

    Q_EMIT enabledChanged();
    return;
  }

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
      tr("Activate Serial Studio Pro or start the free trial to enable MDF4 export."));
}

/**
 * @brief Enqueues one block for export. The single producer for this SPSC queue is the pipeline
 *        thread, for both lanes (spec 0055 D8).
 */
void MDF4::Export::ingestBlock(const DataModel::DataBlockPtr& block)
{
#ifdef BUILD_COMMERCIAL
  if (!block || !exportEnabled() || SerialStudio::isAnyPlayerOpen())
    return;

  enqueueData(block);
#else
  (void)block;
#endif
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Called when the worker's file open state changes.
 */
void MDF4::Export::onWorkerOpenChanged()
{
  auto* worker = static_cast<ExportWorker*>(m_worker);
  m_isOpen.store(worker->isResourceOpen(), std::memory_order_relaxed);
  Q_EMIT openChanged();
}
#endif

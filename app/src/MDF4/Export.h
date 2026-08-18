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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <map>
#include <memory>
#include <QDateTime>
#include <QObject>
#include <QSettings>
#include <vector>

#include "DataModel/DataBlock.h"
#include "DataModel/ExportSchema.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameConsumer.h"

class AppState;

namespace DataModel {
class FrameBuilder;
}  // namespace DataModel

namespace mdf {
class MdfWriter;
class IChannel;
class IChannelGroup;
class IDataGroup;
}  // namespace mdf

namespace MDF4 {
class Export;

#ifdef BUILD_COMMERCIAL
/**
 * @brief Worker that handles MDF4 export file I/O on a background thread.
 */
class ExportWorker : public DataModel::FrameConsumerWorker<DataModel::DataBlockPtr> {
  Q_OBJECT

public:
  ExportWorker(moodycamel::ReaderWriterQueue<DataModel::DataBlockPtr>* queue,
               std::atomic<bool>* enabled,
               std::atomic<size_t>* queueSize);
  ~ExportWorker() override;

  void closeResources() override;
  [[nodiscard]] bool isResourceOpen() const override;

public slots:
  void setTemplateFrame(const DataModel::Frame& frame);
  void applyPublishedStructure(const DataModel::Frame& frame);

public:
  /**
   * @brief Per-channel-group state captured during file creation.
   */
  struct ChannelGroupInfo {
    mdf::IChannelGroup* channelGroup;
    mdf::IChannel* timeChannel;
    std::vector<mdf::IChannel*> channels;
    std::vector<mdf::IChannel*> rawChannels;
    std::vector<bool> isNumeric;
  };

  /**
   * @brief Where one block column's samples land: the owning channel group and its slot in it.
   */
  struct ColumnSlot {
    int groupId       = -1;
    std::size_t index = 0;
  };

protected:
  void processItems(const std::vector<DataModel::DataBlockPtr>& items) override;

private:
  void createFile(const DataModel::Frame& frame);
  void buildColumnMap(const DataModel::Frame& frame);
  void resolveBlockColumns(const DataModel::DataBlock& block);
  void writeBlockSample(const DataModel::DataBlock& block, qsizetype index, qint64 systemEpochNs);
  void buildChannelGroups(mdf::IDataGroup* dataGroup, const DataModel::Frame& frame);
  void buildChannelGroupForGroup(mdf::IDataGroup* dataGroup,
                                 const DataModel::Group& group,
                                 const QString& sourceTitle,
                                 bool usingTemplate,
                                 const std::map<std::pair<int, int>, bool>& numericLookup);
  void addDatasetChannels(mdf::IChannelGroup* channelGroup,
                          const DataModel::Dataset& dataset,
                          bool isNum,
                          ChannelGroupInfo& info);
  [[nodiscard]] bool initWriterAndHeader(const QString& frameName, const QDateTime& dateTime);

private:
  DataModel::Frame m_templateFrame;
  bool m_fileOpen;
  QString m_filePath;
  std::unique_ptr<mdf::MdfWriter> m_writer;
  std::map<int, ChannelGroupInfo> m_groupMap;
  std::map<int, ColumnSlot> m_columnMap;
  std::vector<int> m_touchedGroups;

  /**
   * @brief One block column's resolved MDF4 targets. Both maps are fixed for the file's lifetime,
   *        so resolving per sample repeats the same two tree walks for every sample of the block.
   */
  struct ResolvedColumn {
    mdf::IChannel* channel    = nullptr;
    mdf::IChannel* rawChannel = nullptr;
    bool isNumeric            = true;
    bool valid                = false;
    int groupId               = -1;
  };

  std::vector<ResolvedColumn> m_resolved;

  DataModel::TimestampedFrame::SteadyTimePoint m_steadyBaseline;
  std::chrono::system_clock::time_point m_systemBaseline;
};

#endif

/**
 * @brief Manages automatic export of telemetry data to MDF4 files (Pro only).
 */
class Export
#ifdef BUILD_COMMERCIAL
  : public DataModel::FrameConsumer<DataModel::DataBlockPtr>
#else
  : public QObject
#endif
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool isOpen
             READ isOpen
             NOTIFY openChanged)
  Q_PROPERTY(bool exportEnabled
             READ exportEnabled
             WRITE setExportEnabled
             NOTIFY enabledChanged)
  // clang-format on

signals:
  void openChanged();
  void enabledChanged();

private:
  explicit Export();
  Export(Export&&)                 = delete;
  Export(const Export&)            = delete;
  Export& operator=(Export&&)      = delete;
  Export& operator=(const Export&) = delete;

  ~Export();

public:
  [[nodiscard]] static Export& instance();

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool exportEnabled() const;

public slots:
  void closeFile();
  void setupExternalConnections();
  void setExportEnabled(const bool enabled);
  void setSettingsPersistent(const bool persistent);
  void ingestBlock(const DataModel::DataBlockPtr& block);

protected:
#ifdef BUILD_COMMERCIAL
  DataModel::FrameConsumerWorkerBase* createWorker() override;
#endif

private slots:
#ifdef BUILD_COMMERCIAL
  void onWorkerOpenChanged();
#endif

private:
#ifdef BUILD_COMMERCIAL
  void refreshTemplateFrame();
#endif

private:
  QSettings m_settings;
  std::atomic<bool> m_isOpen;
  std::atomic<bool> m_exportEnabled;
  bool m_persistSettings;
#ifdef BUILD_COMMERCIAL
  DataModel::Frame m_sessionStructure;
#endif
};
}  // namespace MDF4

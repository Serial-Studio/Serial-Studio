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

#include "DataModel/ExportSchema.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameConsumer.h"
#include "IO/StreamWorker.h"

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
class ExportWorker : public DataModel::FrameConsumerWorker<DataModel::TimestampedFramePtr> {
  Q_OBJECT

public:
  ExportWorker(moodycamel::ReaderWriterQueue<DataModel::TimestampedFramePtr>* queue,
               std::atomic<bool>* enabled,
               std::atomic<size_t>* queueSize);
  ~ExportWorker() override;

  void closeResources() override;
  [[nodiscard]] bool isResourceOpen() const override;

public slots:
  void setTemplateFrame(const DataModel::Frame& frame);

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

protected:
  void processItems(const std::vector<DataModel::TimestampedFramePtr>& items) override;

private:
  void createFile(const DataModel::Frame& frame);
  void writeGroupDatasets(const DataModel::Group& group, ChannelGroupInfo& info);
  void writeFrameGroups(const DataModel::Frame& frame, qint64 timestamp_ns, double timestamp_s);
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

  DataModel::TimestampedFrame::SteadyTimePoint m_steadyBaseline;
  std::chrono::system_clock::time_point m_systemBaseline;
};

/**
 * @brief Worker writing full-rate typed stream blocks (spec 0051 M5) to one MDF4 file per
 *        stream source: native float channels, per-sample timestamps derived as t0 + i * dt.
 */
class StreamExportWorker : public DataModel::FrameConsumerWorker<IO::StreamBlockItemPtr> {
  Q_OBJECT

public:
  StreamExportWorker(moodycamel::ReaderWriterQueue<IO::StreamBlockItemPtr>* queue,
                     std::atomic<bool>* enabled,
                     std::atomic<size_t>* queueSize);
  ~StreamExportWorker() override;

  void closeResources() override;
  [[nodiscard]] bool isResourceOpen() const override;

protected:
  void processItems(const std::vector<IO::StreamBlockItemPtr>& items) override;

private:
  struct StreamFile {
    std::unique_ptr<mdf::MdfWriter> writer;
    mdf::IChannelGroup* channelGroup = nullptr;
    mdf::IChannel* timeChannel       = nullptr;
    std::vector<mdf::IChannel*> channels;
    DataModel::TimestampedFrame::SteadyTimePoint steadyBaseline;
    std::chrono::system_clock::time_point systemBaseline;
  };

  [[nodiscard]] StreamFile* fileFor(const IO::StreamBlockItem& block);
  void writeBlock(StreamFile& state, const IO::StreamBlockItem& block);

private:
  std::map<int, StreamFile> m_files;
};

/**
 * @brief FrameConsumer facade for the stream-block MDF4 sink; the single producer is the GUI
 *        thread (ingestBlock is a queued slot fed by every StreamProcessor's blockReady).
 */
class StreamExport : public DataModel::FrameConsumer<IO::StreamBlockItemPtr> {
  Q_OBJECT

public:
  explicit StreamExport();

public slots:
  void ingestBlock(const IO::StreamBlockItemPtr& block);
  void closeFiles();

protected:
  DataModel::FrameConsumerWorkerBase* createWorker() override;
};
#endif

/**
 * @brief Manages automatic export of telemetry data to MDF4 files (Pro only).
 */
class Export
#ifdef BUILD_COMMERCIAL
  : public DataModel::FrameConsumer<DataModel::TimestampedFramePtr>
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
#ifdef BUILD_COMMERCIAL
  [[nodiscard]] StreamExport& streamSink() noexcept;
#endif

public slots:
  void closeFile();
  void setupExternalConnections();
  void setExportEnabled(const bool enabled);
  void setSettingsPersistent(const bool persistent);
  void hotpathTxFrame(const DataModel::TimestampedFramePtr& frame);

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
  AppState* m_appState;
  DataModel::FrameBuilder* m_frameBuilder;
  StreamExport m_streamExport;
#endif
};
}  // namespace MDF4

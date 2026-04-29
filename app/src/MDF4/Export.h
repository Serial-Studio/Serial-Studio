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

#pragma once

#include <map>
#include <memory>
#include <QObject>
#include <QSettings>
#include <vector>

#include "DataModel/ExportSchema.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameConsumer.h"

namespace mdf {
class MdfWriter;
class IChannel;
class IChannelGroup;
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
  bool isResourceOpen() const override;

protected:
  void processItems(const std::vector<DataModel::TimestampedFramePtr>& items) override;

private:
  void createFile(const DataModel::Frame& frame);

private:
  struct ChannelGroupInfo {
    mdf::IChannelGroup* channelGroup;
    mdf::IChannel* timeChannel;
    std::vector<mdf::IChannel*> channels;
    std::vector<mdf::IChannel*> rawChannels;
    std::vector<bool> isNumeric;
  };

public:
  DataModel::Frame m_templateFrame;

private:
  bool m_fileOpen;
  QString m_filePath;
  std::unique_ptr<mdf::MdfWriter> m_writer;
  std::map<int, ChannelGroupInfo> m_groupMap;

  DataModel::TimestampedFrame::SteadyTimePoint m_steadyBaseline;
  std::chrono::system_clock::time_point m_systemBaseline;
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
  QSettings m_settings;
  std::atomic<bool> m_isOpen;
  std::atomic<bool> m_exportEnabled;
  bool m_persistSettings;
};
}  // namespace MDF4

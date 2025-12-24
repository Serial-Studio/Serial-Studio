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
#include <atomic>
#include <memory>
#include <vector>

#include <QObject>
#include <QThread>
#include <QSettings>

#include "JSON/Frame.h"
#include "ThirdParty/readerwriterqueue.h"

namespace mdf
{
class MdfWriter;
class IChannel;
class IChannelGroup;
} // namespace mdf

namespace MDF4
{
static constexpr size_t kMDF4ExportQueueCapacity = 8192;
static constexpr size_t kMDF4FlushThreshold = 1024;

/**
 * @brief Represents a single timestamped frame for MDF4 export.
 */
struct TimestampFrame
{
  JSON::Frame data;
  QDateTime rxDateTime;

  TimestampFrame() = default;

  TimestampFrame(JSON::Frame &&d)
    : data(std::move(d))
    , rxDateTime(QDateTime::currentDateTime())
  {
  }

  TimestampFrame(TimestampFrame &&) = default;
  TimestampFrame(const TimestampFrame &) = delete;
  TimestampFrame &operator=(TimestampFrame &&) = default;
  TimestampFrame &operator=(const TimestampFrame &) = delete;
};

class Export;

#ifdef BUILD_COMMERCIAL
/**
 * @brief Worker that handles MDF4 export file I/O on background thread
 */
class ExportWorker : public QObject
{
  Q_OBJECT

public:
  explicit ExportWorker(moodycamel::ReaderWriterQueue<TimestampFrame> *queue,
                        std::atomic<bool> *exportEnabled,
                        std::atomic<size_t> *queueSize);
  ~ExportWorker();

  [[nodiscard]] bool isOpen() const;

signals:
  void openChanged();

public slots:
  void writeValues();
  void closeFile();

private:
  void createFile(const JSON::Frame &frame);

private:
  struct ChannelGroupInfo
  {
    mdf::IChannelGroup *channelGroup;
    std::vector<mdf::IChannel *> channels;
    std::vector<bool> isNumeric;
  };

  bool m_fileOpen;
  QString m_filePath;
  std::unique_ptr<mdf::MdfWriter> m_writer;
  std::vector<TimestampFrame> m_writeBuffer;
  std::map<int, ChannelGroupInfo> m_groupMap;
  mdf::IChannel *m_masterTimeChannel;
  moodycamel::ReaderWriterQueue<TimestampFrame> *m_pendingFrames;
  std::atomic<bool> *m_exportEnabled;
  std::atomic<size_t> *m_queueSize;
};
#endif

/**
 * @class Export
 * @brief Manages automatic export of telemetry data to MDF4 files.
 *
 * The Export class is a singleton that provides functionality to capture
 * and export telemetry data to MDF4/MF4 binary measurement files. This is
 * particularly useful for data logging, analysis with automotive tools,
 * and post-processing with professional measurement software.
 *
 * Key Features:
 * - **Binary Format**: Uses industry-standard MDF4 format
 * - **Hierarchical Structure**: One channel group per Serial Studio group
 * - **Efficient Storage**: Compressed binary storage for large datasets
 * - **Tool Compatibility**: Compatible with Vector CANape, ETAS INCA, etc.
 * - **Buffered Writing**: Buffers frames and writes periodically to reduce
 *   disk I/O
 * - **Pro Feature**: Available only in commercial builds with valid license
 * - **Singleton Pattern**: Single instance ensures consistent file handling
 *   across the application
 *
 * @note This feature is only available in commercial builds (BUILD_COMMERCIAL).
 *       In GPL builds, all methods return false/empty values and no export
 *       occurs.
 *
 * @warning Export functionality requires an active Serial Studio Pro license.
 *          The export will be automatically disabled if the license becomes
 *          invalid.
 */
class Export : public QObject
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
  Export(Export &&) = delete;
  Export(const Export &) = delete;
  Export &operator=(Export &&) = delete;
  Export &operator=(const Export &) = delete;

  ~Export();

public:
  static Export &instance();

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool exportEnabled() const;

public slots:
  void closeFile();
  void setupExternalConnections();
  void setExportEnabled(const bool enabled);
  void hotpathTxFrame(const JSON::Frame &frame);

private slots:
#ifdef BUILD_COMMERCIAL
  void onWorkerOpenChanged();
#endif

private:
  QSettings m_settings;
  std::atomic<bool> m_isOpen;
  std::atomic<bool> m_exportEnabled;

#ifdef BUILD_COMMERCIAL
  QThread m_workerThread;
  ExportWorker *m_worker;
  std::atomic<size_t> m_queueSize;
  moodycamel::ReaderWriterQueue<TimestampFrame> m_pendingFrames{
      kMDF4ExportQueueCapacity};
#endif
};
} // namespace MDF4

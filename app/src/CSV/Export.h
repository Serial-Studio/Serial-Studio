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

#include <QFile>
#include <QObject>
#include <QSet>
#include <QSettings>
#include <QTextStream>
#include <QTimer>
#include <QVector>

#include "DataModel/ExportSchema.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameConsumer.h"
#include "IO/StreamWorker.h"

namespace CSV {
class Export;

/**
 * @brief Worker writing full-rate typed stream blocks (spec 0051 M5) to one numeric CSV per
 *        stream source; per-sample timestamps derive as t0 + i * dt, never re-stamped.
 */
class StreamExportWorker : public DataModel::FrameConsumerWorker<IO::StreamBlockItemPtr> {
  Q_OBJECT

public:
  StreamExportWorker(moodycamel::ReaderWriterQueue<IO::StreamBlockItemPtr>* queue,
                     std::atomic<bool>* enabled,
                     std::atomic<size_t>* queueSize);

  void closeResources() override;
  [[nodiscard]] bool isResourceOpen() const override;

protected:
  void processItems(const std::vector<IO::StreamBlockItemPtr>& items) override;

private:
  struct FileState {
    std::unique_ptr<QFile> file;
    std::unique_ptr<QTextStream> stream;
    DataModel::TimestampedFrame::SteadyTimePoint start;
  };

  [[nodiscard]] FileState* fileFor(const IO::StreamBlockItem& block);
  void writeBlock(FileState& state, const IO::StreamBlockItem& block);

private:
  std::map<int, FileState> m_files;
};

/**
 * @brief FrameConsumer facade for the stream-block CSV sink; the single producer is the GUI
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

/**
 * @brief Worker object that performs CSV file I/O on a background thread.
 */
class ExportWorker : public DataModel::FrameConsumerWorker<DataModel::TimestampedFramePtr> {
  Q_OBJECT

public:
  ExportWorker(moodycamel::ReaderWriterQueue<DataModel::TimestampedFramePtr>* queue,
               std::atomic<bool>* enabled,
               std::atomic<size_t>* queueSize);

  void closeResources() override;
  [[nodiscard]] bool isResourceOpen() const override;

public slots:
  void writeSnapshotRow();
  void setSnapshotIntervalMs(int interval);
  void setTemplateFrame(const DataModel::Frame& frame);

protected:
  void processItems(const std::vector<DataModel::TimestampedFramePtr>& items) override;

private:
  void createCsvFile(const DataModel::Frame& frame);
  void writeRow(const DataModel::TimestampedFrame::SteadyTimePoint& timestamp);

private:
  int m_snapshotIntervalMs;
  QTimer* m_snapshotTimer;

  DataModel::Frame m_templateFrame;
  QFile m_csvFile;
  QTextStream m_textStream;
  DataModel::ExportSchema m_schema;
  DataModel::TimestampedFrame::SteadyTimePoint m_referenceTimestamp;

  QMap<int, QString> m_lastFinalValues;
};

/**
 * @brief Handles CSV export of incoming data frames.
 */
class Export : public DataModel::FrameConsumer<DataModel::TimestampedFramePtr> {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool isOpen
             READ isOpen
             NOTIFY openChanged)
  Q_PROPERTY(bool exportEnabled
             READ exportEnabled
             WRITE setExportEnabled
             NOTIFY enabledChanged)
  Q_PROPERTY(int exportInterval
             READ exportInterval
             WRITE setExportInterval
             NOTIFY intervalChanged)
  // clang-format on

signals:
  void openChanged();
  void enabledChanged();
  void intervalChanged();

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
  [[nodiscard]] int exportInterval() const;
  [[nodiscard]] StreamExport& streamSink() noexcept;

public slots:
  void closeFile();
  void setupExternalConnections();
  void setExportEnabled(const bool enabled);
  void setExportInterval(const int interval);
  void setSettingsPersistent(const bool persistent);

  void hotpathTxFrame(const DataModel::TimestampedFramePtr& frame);

protected:
  DataModel::FrameConsumerWorkerBase* createWorker() override;

private slots:
  void onWorkerOpenChanged();

private:
  QSettings m_settings;
  std::atomic<bool> m_isOpen;
  bool m_persistSettings;
  int m_exportInterval;
  StreamExport m_streamExport;
};
}  // namespace CSV

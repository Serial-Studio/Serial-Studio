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

#include <QDir>
#include <QFile>
#include <QObject>
#include <QSet>
#include <QSettings>
#include <QTextStream>
#include <QTimer>
#include <QVector>

#include "Core/Bus/Subscription.h"
#include "Core/DataModel/DataBlock.h"
#include "Core/DataModel/ExportSchema.h"
#include "Core/DataModel/Frame.h"
#include "Core/DataModel/FrameConsumer.h"
#include "CSV/SparseRowMerger.h"
#include "DataModel/ExportStructure.h"

namespace Core::Bus {
class MessageBus;
}  // namespace Core::Bus

namespace CSV {
class Export;

/**
 * @brief Worker that performs CSV file I/O on a background thread. One file per session carries
 *        every source (spec 0055 R6): rows are SPARSE -- one row per distinct sample instant, with
 *        cells filled only for the datasets sampled at that instant and nothing forward-filled --
 *        and a bounded reorder window (D3/D7) keeps them strictly time-ordered across sources.
 */
class ExportWorker : public DataModel::FrameConsumerWorker<DataModel::DataBlockPtr> {
  Q_OBJECT

public:
  ExportWorker(moodycamel::ReaderWriterQueue<DataModel::DataBlockPtr>* queue,
               std::atomic<bool>* enabled,
               std::atomic<size_t>* queueSize);

  void closeResources() override;
  [[nodiscard]] bool isResourceOpen() const override;

public slots:
  void writeSnapshotRow();
  void setSnapshotIntervalMs(int interval);
  void setTemplateFrame(const DataModel::Frame& frame);
  void applyPublishedStructure(const DataModel::Frame& frame);

protected:
  void processItems(const std::vector<DataModel::DataBlockPtr>& items) override;

private:
  void createCsvFile(const DataModel::Frame& frame);
  [[nodiscard]] bool openUniqueFile(const QDir& dir, const QString& base);
  void bufferBlock(const DataModel::DataBlockPtr& block);
  void flushReadyRows(qint64 cutoffNs);
  void writeSnapshotRowNow(const DataModel::TimestampedFrame::SteadyTimePoint& timestamp);

private:
  int m_snapshotIntervalMs;
  QTimer* m_snapshotTimer;

  DataModel::ExportStructure m_structure;
  QFile m_csvFile;
  QByteArray m_rowBuffer;
  DataModel::ExportSchema m_schema;
  DataModel::TimestampedFrame::SteadyTimePoint m_referenceTimestamp;

  CSV::SparseRowMerger m_merger;
  QMap<int, QString> m_lastFinalValues;
};

/**
 * @brief Handles CSV export of incoming data frames.
 */
class Export : public DataModel::FrameConsumer<DataModel::DataBlockPtr> {
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
  void attachMessageBus(Core::Bus::MessageBus& bus);

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool exportEnabled() const;
  [[nodiscard]] bool sinkActive() const noexcept override;
  [[nodiscard]] int exportInterval() const;

public slots:
  void closeFile();
  void setupExternalConnections();
  void setExportEnabled(const bool enabled);
  void setExportInterval(const int interval);
  void setSettingsPersistent(const bool persistent);

  void ingestBlock(const DataModel::DataBlockPtr& block) override;

protected:
  DataModel::FrameConsumerWorkerBase* createWorker() override;

private slots:
  void onWorkerOpenChanged();

private:
  QSettings m_settings;
  std::atomic<bool> m_isOpen;
  bool m_persistSettings;
  int m_exportInterval;
  Core::Bus::MessageBus* m_bus;
  Core::Bus::Subscription m_operationModeWatch;
};
}  // namespace CSV

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

#include <QFile>
#include <QObject>
#include <QTextStream>
#include <QVector>

#include "DataModel/Frame.h"
#include "DataModel/FrameConsumer.h"

namespace CSV {
class Export;

/**
 * @brief Worker object that performs CSV file I/O on a background thread.
 *
 * This class owns all file-related resources and performs disk writes
 * entirely on a dedicated worker thread to avoid blocking the main UI thread.
 */
class ExportWorker : public DataModel::FrameConsumerWorker<DataModel::TimestampedFramePtr> {
  Q_OBJECT

public:
  using DataModel::FrameConsumerWorker<DataModel::TimestampedFramePtr>::FrameConsumerWorker;

  void closeResources() override;
  bool isResourceOpen() const override;

protected:
  void processItems(const std::vector<DataModel::TimestampedFramePtr>& items) override;

private:
  QVector<QPair<int, QString>> createCsvFile(const DataModel::Frame& frame);

private:
  QFile m_csvFile;
  QTextStream m_textStream;
  QVector<QPair<int, QString>> m_indexHeaderPairs;
  DataModel::TimestampedFrame::SteadyTimePoint m_referenceTimestamp;
};

/**
 * @brief Handles CSV export of incoming data frames.
 *
 * The Export class collects incoming JSON frames tagged with a timestamp,
 * writes them asynchronously to a CSV file, and manages output buffering
 * and formatting.
 *
 * This class is implemented as a singleton and runs a background thread
 * to offload file I/O operations. It supports enabling/disabling export
 * dynamically and integrates with external modules (IO manager, MQTT, etc.).
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
  static Export& instance();

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool exportEnabled() const;

public slots:
  void closeFile();
  void setupExternalConnections();
  void setExportEnabled(const bool enabled);

  void hotpathTxFrame(const DataModel::TimestampedFramePtr& frame);

protected:
  DataModel::FrameConsumerWorkerBase* createWorker() override;

private slots:
  void onWorkerOpenChanged();

private:
  std::atomic<bool> m_isOpen;
};
}  // namespace CSV

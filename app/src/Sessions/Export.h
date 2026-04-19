/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <atomic>
#include <QMutex>
#include <QObject>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "DataModel/ExportSchema.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameConsumer.h"
#include "IO/ConnectionManager.h"

#ifdef BUILD_COMMERCIAL

namespace Sessions {
class Export;

//--------------------------------------------------------------------------------------------------
// Timestamped raw bytes for the second queue
//--------------------------------------------------------------------------------------------------

/**
 * @brief Lightweight struct for raw console bytes with timestamp.
 */
struct TimestampedRawBytes {
  int deviceId;
  IO::ByteArrayPtr data;
  DataModel::TimestampedFrame::SteadyTimePoint timestamp;
};

//--------------------------------------------------------------------------------------------------
// SQLite export worker
//--------------------------------------------------------------------------------------------------

/**
 * @brief Worker that writes telemetry data and raw bytes to a SQLite database
 *        on a background thread.
 *
 * Drains two lock-free queues per processData() call: the frame queue (via
 * FrameConsumerWorker) and a second queue for raw console bytes. Both are
 * written within a single transaction for consistency and performance.
 */
class ExportWorker : public DataModel::FrameConsumerWorker<DataModel::TimestampedFramePtr> {
  Q_OBJECT

public:
  ExportWorker(moodycamel::ReaderWriterQueue<DataModel::TimestampedFramePtr>* frameQueue,
               std::atomic<bool>* enabled,
               std::atomic<size_t>* queueSize,
               moodycamel::ReaderWriterQueue<TimestampedRawBytes>* rawQueue,
               std::atomic<int>* operationMode,
               QMutex* projectSnapshotMutex,
               const QByteArray* projectSnapshot);
  ~ExportWorker() override;

  void closeResources() override;
  bool isResourceOpen() const override;
  void processData() override;

protected:
  void processItems(const std::vector<DataModel::TimestampedFramePtr>& items) override;

private:
  void createDatabase(const DataModel::Frame& frame);
  void createSchema(QSqlQuery& q);
  void insertSession(const DataModel::Frame& frame, const QDateTime& dt);
  void writeColumnDefs(const DataModel::Frame& frame);
  void storeProjectMetadata(const DataModel::Frame& frame);
  void prepareHotpathQueries();
  void writeRawBytes();
  void finalizeSession();

  // Produces a replayable project JSON for the row we're about to insert.
  // Prefers the main-thread snapshot staged by Sessions::Export; falls back
  // to synthesising a minimal project from the live frame when no snapshot
  // is available (QuickPlot mode, or Console-only) so Sessions::Player can
  // still restore the widget layout on replay.
  [[nodiscard]] QJsonObject buildReplayProjectJson(const DataModel::Frame& frame) const;

private:
  bool m_dbOpen;
  int m_sessionId;
  QSqlDatabase m_db;
  DataModel::ExportSchema m_schema;
  DataModel::TimestampedFrame::SteadyTimePoint m_steadyBaseline;

  QSqlQuery m_readingQuery;
  QSqlQuery m_rawBytesQuery;
  QSqlQuery m_tableSnapshotQuery;

  moodycamel::ReaderWriterQueue<TimestampedRawBytes>* m_rawQueue;
  std::atomic<int>* m_operationMode;

  // Shared main-thread/worker-thread snapshot of ProjectModel::serializeToJson().
  // The pointer and mutex are owned by Sessions::Export; the worker reads via
  // the singleton without touching ProjectModel across threads.
  QMutex* m_projectSnapshotMutex;
  const QByteArray* m_projectSnapshot;
};

//--------------------------------------------------------------------------------------------------
// SQLite export manager
//--------------------------------------------------------------------------------------------------

/**
 * @brief Singleton managing SQLite export of telemetry data (Pro only).
 *
 * Stores multiple sessions per project database. Each session records:
 * - Column definitions (from ExportSchema)
 * - Readings with raw and transformed values (numeric + string)
 * - Raw console bytes (from ConnectionManager)
 * - Data table snapshots (user-defined tables + computed values)
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
  [[nodiscard]] static Export& instance();

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool exportEnabled() const;

public slots:
  void closeFile();
  void setupExternalConnections();
  void setExportEnabled(const bool enabled);
  void hotpathTxFrame(const DataModel::TimestampedFramePtr& frame);
  void hotpathTxRawBytes(int deviceId, const IO::ByteArrayPtr& data);

protected:
  DataModel::FrameConsumerWorkerBase* createWorker() override;

private slots:
  void onWorkerOpenChanged();

private:
  // Snapshots the current project JSON from the main thread so the worker
  // can build the replayable session row without touching ProjectModel
  // across threads.
  void refreshProjectSnapshot();

private:
  QSettings m_settings;
  std::atomic<bool> m_isOpen;
  std::atomic<bool> m_exportEnabled;

  moodycamel::ReaderWriterQueue<TimestampedRawBytes> m_rawBytesQueue;
  std::atomic<int> m_operationMode;

  // Main-thread-written, worker-thread-read. The worker's ExportWorker holds
  // borrowed pointers to both members so it can read under the mutex without
  // re-walking the Export singleton pointer chain.
  QMutex m_projectSnapshotMutex;
  QByteArray m_projectSnapshot;
};

}  // namespace Sessions

#endif  // BUILD_COMMERCIAL

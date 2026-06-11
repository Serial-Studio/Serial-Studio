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
#include <cstddef>
#include <new>
#include <optional>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "DataModel/DataTable.h"
#include "DataModel/ExportSchema.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameConsumer.h"
#include "IO/ConnectionManager.h"

#ifdef BUILD_COMMERCIAL

namespace Sessions {
class Export;

/**
 * @brief Raw driver bytes paired with device id and capture timestamp.
 */
struct TimestampedRawBytes {
  int deviceId;
  IO::CapturedDataPtr data;
};

/**
 * @brief One changed data-table register captured for the table_snapshots table.
 */
struct TableSnapshotEntry {
  DataModel::TimestampedFrame::SteadyTimePoint timestamp;
  QString tableName;
  QString registerName;
  DataModel::RegisterValue value;
};

/**
 * @brief Background worker that persists frames and raw bytes to SQLite.
 */
class ExportWorker : public DataModel::FrameConsumerWorker<DataModel::TimestampedFramePtr> {
  Q_OBJECT

signals:
  void sessionIdAssigned(int sessionId);

public:
  ExportWorker(moodycamel::ReaderWriterQueue<DataModel::TimestampedFramePtr>* frameQueue,
               std::atomic<bool>* enabled,
               std::atomic<size_t>* queueSize,
               moodycamel::ReaderWriterQueue<TimestampedRawBytes>* rawQueue,
               moodycamel::ReaderWriterQueue<TableSnapshotEntry>* snapshotQueue,
               std::atomic<int>* operationMode,
               QMutex* projectSnapshotMutex,
               const QByteArray* projectSnapshot);
  ~ExportWorker() override;

  void closeResources() override;
  [[nodiscard]] bool isResourceOpen() const override;
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
  void writeTableSnapshots();
  void writeFrameReadings(const DataModel::TimestampedFramePtr& frame);
  void bindAndInsertReading(qint64 ns, const DataModel::Dataset& dataset);
  void finalizeSession();

  [[nodiscard]] QJsonObject buildReplayProjectJson(const DataModel::Frame& frame) const;

private:
  bool m_dbOpen;
  int m_sessionId;
  std::optional<QSqlDatabase> m_db;
  DataModel::ExportSchema m_schema;
  DataModel::TimestampedFrame::SteadyTimePoint m_steadyBaseline;
  qint64 m_lastRawBytesNs;

  std::optional<QSqlQuery> m_readingQuery;
  std::optional<QSqlQuery> m_rawBytesQuery;
  std::optional<QSqlQuery> m_tableSnapshotQuery;

  moodycamel::ReaderWriterQueue<TimestampedRawBytes>* m_rawQueue;
  moodycamel::ReaderWriterQueue<TableSnapshotEntry>* m_snapshotQueue;
  std::atomic<int>* m_operationMode;
  QMutex* m_projectSnapshotMutex;
  const QByteArray* m_projectSnapshot;
};

/**
 * @brief Session-database export controller (Pro) driving the SQLite worker.
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
  Q_PROPERTY(int currentSessionId
             READ currentSessionId
             NOTIFY currentSessionIdChanged)
  // clang-format on

signals:
  void openChanged();
  void enabledChanged();
  void currentSessionIdChanged();

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
  [[nodiscard]] int currentSessionId() const;

public slots:
  void closeFile();
  void setupExternalConnections();
  void setExportEnabled(const bool enabled);
  void setSettingsPersistent(const bool persistent);
  void hotpathTxFrame(const DataModel::TimestampedFramePtr& frame);
  void hotpathTxRawBytes(int deviceId, const IO::CapturedDataPtr& data);

protected:
  DataModel::FrameConsumerWorkerBase* createWorker() override;

private slots:
  void onWorkerOpenChanged();
  void captureTableSnapshots();
  void onWorkerSessionIdAssigned(int sessionId);

private:
  void refreshProjectSnapshot();

private:
  static constexpr std::size_t kCacheLine = 64;

  QSettings m_settings;
  alignas(kCacheLine) std::atomic<bool> m_isOpen;
  alignas(kCacheLine) std::atomic<bool> m_exportEnabled;
  alignas(kCacheLine) std::atomic<int> m_currentSessionId;
  bool m_persistSettings;

  moodycamel::ReaderWriterQueue<TimestampedRawBytes> m_rawBytesQueue;
  moodycamel::ReaderWriterQueue<TableSnapshotEntry> m_tableSnapshotQueue;
  alignas(kCacheLine) std::atomic<int> m_operationMode;

  QMap<QString, QMap<QString, DataModel::RegisterValue>> m_lastTableSnapshot;

  QMutex m_projectSnapshotMutex;
  QByteArray m_projectSnapshot;
};

}  // namespace Sessions

#endif  // BUILD_COMMERCIAL

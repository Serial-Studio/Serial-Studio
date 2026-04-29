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

/**
 * @brief Raw driver bytes paired with device id and capture timestamp.
 */
struct TimestampedRawBytes {
  int deviceId;
  IO::CapturedDataPtr data;
};

/**
 * @brief Background worker that persists frames and raw bytes to SQLite.
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

  QJsonObject buildReplayProjectJson(const DataModel::Frame& frame) const;

private:
  bool m_dbOpen;
  int m_sessionId;
  QSqlDatabase m_db;
  DataModel::ExportSchema m_schema;
  DataModel::TimestampedFrame::SteadyTimePoint m_steadyBaseline;
  qint64 m_lastRawBytesNs;

  QSqlQuery m_readingQuery;
  QSqlQuery m_rawBytesQuery;
  QSqlQuery m_tableSnapshotQuery;

  moodycamel::ReaderWriterQueue<TimestampedRawBytes>* m_rawQueue;
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
  void hotpathTxRawBytes(int deviceId, const IO::CapturedDataPtr& data);

protected:
  DataModel::FrameConsumerWorkerBase* createWorker() override;

private slots:
  void onWorkerOpenChanged();

private:
  void refreshProjectSnapshot();

private:
  QSettings m_settings;
  std::atomic<bool> m_isOpen;
  std::atomic<bool> m_exportEnabled;
  bool m_persistSettings;

  moodycamel::ReaderWriterQueue<TimestampedRawBytes> m_rawBytesQueue;
  std::atomic<int> m_operationMode;

  QMutex m_projectSnapshotMutex;
  QByteArray m_projectSnapshot;
};

}  // namespace Sessions

#endif  // BUILD_COMMERCIAL

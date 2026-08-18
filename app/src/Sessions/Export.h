/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <atomic>
#include <cstddef>
#include <new>
#include <optional>
#include <QCryptographicHash>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTimer>

#include "DataModel/DataBlock.h"
#include "DataModel/DataTable.h"
#include "DataModel/ExportSchema.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameConsumer.h"
#include "IO/ConnectionManager.h"

class AppState;

namespace DataModel {
class ControlScript;
class FrameBuilder;
class ProjectModel;
}  // namespace DataModel

#ifdef BUILD_COMMERCIAL

namespace UI {
class Dashboard;
}  // namespace UI

namespace Sessions {
class Export;

/**
 * @brief Feeds one raw_bytes row into a fingerprint hash using the spec-0044 canonical layout.
 */
void hashRawChunk(QCryptographicHash& hash, qint64 ns, int deviceId, const QByteArray& data);

/**
 * @brief Feeds one readings row into a fingerprint hash using the spec-0044 canonical layout.
 */
void hashReadingRow(QCryptographicHash& hash,
                    qint64 ns,
                    qint64 uniqueId,
                    double rawNumeric,
                    const QString& rawString,
                    double finalNumeric,
                    const QString& finalString,
                    bool isNumeric);

/**
 * @brief Feeds one `blocks` row into a fingerprint hash (spec 0055). Both blobs are already
 *        canonical little-endian, so the digest is machine-independent.
 */
void hashBlockRow(QCryptographicHash& hash,
                  qint64 uniqueId,
                  qint64 t0Ns,
                  qint64 dtNs,
                  qint64 frames,
                  const QByteArray& values,
                  const QByteArray& rawValues,
                  const QByteArray& texts);

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
class ExportWorker : public DataModel::FrameConsumerWorker<DataModel::DataBlockPtr> {
  Q_OBJECT

signals:
  void sessionIdAssigned(int sessionId);

public:
  ExportWorker(moodycamel::ReaderWriterQueue<DataModel::DataBlockPtr>* blockQueue,
               std::atomic<bool>* enabled,
               std::atomic<size_t>* queueSize,
               moodycamel::ReaderWriterQueue<TimestampedRawBytes>* rawQueue,
               moodycamel::ReaderWriterQueue<TableSnapshotEntry>* snapshotQueue,
               std::atomic<int>* operationMode,
               QMutex* projectSnapshotMutex,
               const QByteArray* projectSnapshot,
               const QByteArray* viewStateSnapshot,
               const std::atomic<bool>* controlScriptSeen,
               const std::atomic<quint64>* linkDroppedFrames,
               const std::atomic<quint64>* linkOverflowBytes);
  ~ExportWorker() override;

  void closeResources() override;
  [[nodiscard]] bool isResourceOpen() const override;
  void processData() override;

public slots:
  void setTemplateFrame(const DataModel::Frame& frame);
  void applyPublishedStructure(const DataModel::Frame& frame);
  void storeViewState();

protected:
  void processItems(const std::vector<DataModel::DataBlockPtr>& items) override;

private:
  void createDatabase(const DataModel::Frame& frame);
  void createSchema(QSqlQuery& q);
  void insertSession(const DataModel::Frame& frame, const QDateTime& dt);
  void writeColumnDefs(const DataModel::Frame& frame);
  void storeProjectMetadata(const DataModel::Frame& frame);
  void prepareHotpathQueries();
  void writeRawBytes();
  void writeTableSnapshots();
  void writeBlocks(const DataModel::DataBlockPtr& block);
  void insertBlockRow(const DataModel::DataBlock& block,
                      const DataModel::BlockColumn& column,
                      qint64 t0Ns,
                      qint64 dtNs);
  void finalizeSession();

  [[nodiscard]] QJsonObject buildReplayProjectJson(const DataModel::Frame& frame) const;
  [[nodiscard]] QString buildReproClassJson() const;

private:
  bool m_dbOpen;
  int m_sessionId;
  DataModel::Frame m_templateFrame;
  std::optional<QSqlDatabase> m_db;
  DataModel::ExportSchema m_schema;
  DataModel::TimestampedFrame::SteadyTimePoint m_steadyBaseline;
  qint64 m_lastRawBytesNs;
  QCryptographicHash m_rawHash;
  QCryptographicHash m_blocksHash;

  std::optional<QSqlQuery> m_blockQuery;
  std::optional<QSqlQuery> m_rawBytesQuery;
  std::optional<QSqlQuery> m_tableSnapshotQuery;

  moodycamel::ReaderWriterQueue<TimestampedRawBytes>* m_rawQueue;
  moodycamel::ReaderWriterQueue<TableSnapshotEntry>* m_snapshotQueue;
  std::atomic<int>* m_operationMode;
  QMutex* m_projectSnapshotMutex;
  const QByteArray* m_projectSnapshot;
  const QByteArray* m_viewStateSnapshot;
  const std::atomic<bool>* m_controlScriptSeen;
  const std::atomic<quint64>* m_linkDroppedFrames;
  const std::atomic<quint64>* m_linkOverflowBytes;
};

/**
 * @brief Session-database export controller (Pro) driving the SQLite worker.
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
  void ingestBlock(const DataModel::DataBlockPtr& block);
  void hotpathTxRawBytes(int deviceId, const IO::CapturedDataPtr& data);

protected:
  DataModel::FrameConsumerWorkerBase* createWorker() override;

private slots:
  void onWorkerOpenChanged();
  void captureTableSnapshots();
  void onWorkerSessionIdAssigned(int sessionId);
  void refreshViewStateSnapshot();
  void pushViewStateToWorker();

private:
  void wireViewState();
  void refreshProjectSnapshot();
  void refreshTemplateFrame();
  void resetSessionHealthBaseline();
  void sampleSessionHealth();

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
  QByteArray m_viewStateSnapshot;
  QTimer m_viewStateDebounce;

  alignas(kCacheLine) std::atomic<bool> m_controlScriptSeen;
  alignas(kCacheLine) std::atomic<quint64> m_linkDroppedFrames;
  alignas(kCacheLine) std::atomic<quint64> m_linkOverflowBytes;
  quint64 m_lastLinkDroppedSample;
  quint64 m_lastLinkOverflowSample;

  AppState* m_appState;
  DataModel::ProjectModel* m_projectModel;
  DataModel::FrameBuilder* m_frameBuilder;
  DataModel::Frame m_sessionStructure;
  DataModel::ControlScript* m_controlScript;
  IO::ConnectionManager* m_connectionManager;
  UI::Dashboard* m_dashboard;
};

}  // namespace Sessions

#endif  // BUILD_COMMERCIAL

/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <atomic>
#  include <map>
#  include <QByteArray>
#  include <QHash>
#  include <QJsonObject>
#  include <QList>
#  include <QObject>
#  include <QPointer>
#  include <QSslError>
#  include <QString>
#  include <QUrl>

#  include "DataModel/DataBlock.h"
#  include "DataModel/Frame.h"
#  include "DataModel/FrameConsumer.h"
#  include "InfluxDB/LineProtocol.h"
#  include "MQTT/CredentialVault.h"

class QNetworkAccessManager;
class QNetworkReply;

namespace InfluxDB {

/**
 * @brief Everything the worker needs to build one write request, snapshotted on the GUI thread.
 */
struct SinkConfig {
  bool enabled = false;
  QString url;
  QString org;
  QString bucket;
  QString measurement;
  QString token;
};

/**
 * @brief One dataset's render cache: the field key escaped once at bind, plus the field type
 *        latched from the first block so InfluxDB's first-write type is never contradicted later.
 */
struct RenderField {
  QByteArray key;
  bool isString  = false;
  bool typeKnown = false;
};

/**
 * @brief A source's published structure plus its per-dataset render cache keyed by uniqueId, so
 *        the field keys and their line-protocol type are computed once per bind, not per sample.
 */
struct RenderTemplate {
  DataModel::FrameTemplate tpl;
  QHash<int, RenderField> fields;
};

/**
 * @brief Renders published blocks into line protocol and POSTs them to an InfluxDB 2.x write
 *        endpoint. Its QNetworkAccessManager is created on this thread in bootstrap(), never
 *        moved here. One request may be outstanding: a batch ready while one still is gets
 *        dropped and counted, since acquisition must never wait on a slow server (R33).
 */
class ExportWorker : public DataModel::FrameConsumerWorker<DataModel::DataBlockPtr> {
  Q_OBJECT

signals:
  void errorOccurred(const QString& message);

public:
  ExportWorker(moodycamel::ReaderWriterQueue<DataModel::DataBlockPtr>* queue,
               std::atomic<bool>* enabled,
               std::atomic<size_t>* queueSize,
               std::atomic<quint64>* pointsWritten,
               std::atomic<quint64>* pointsDropped,
               std::atomic<quint64>* fieldsSkipped,
               std::atomic<quint64>* httpErrors);
  ~ExportWorker() override;

  void closeResources() override;
  [[nodiscard]] bool isResourceOpen() const override;

public slots:
  void bootstrap();
  void applyConfig(const InfluxDB::SinkConfig& config);
  void setTemplateFrame(int sourceId, const DataModel::Frame& frame);

protected:
  void processItems(const std::vector<DataModel::DataBlockPtr>& items) override;

private slots:
  void onReplyFinished();
  void onSslErrors(const QList<QSslError>& errors);

private:
  void renderBlock(const DataModel::DataBlock& block);
  void renderSample(const DataModel::DataBlock& block,
                    RenderTemplate& tpl,
                    const QByteArray& head,
                    qsizetype index);
  void flushBatch();
  void dropBatch();
  void noteHttpFailure(const QString& message);
  void rebuildWriteCache();
  void sampleEpochOffset();

  static void buildRenderFields(RenderTemplate& tpl);

  [[nodiscard]] QString failureMessage(QNetworkReply& reply, int status);
  [[nodiscard]] bool configured() const;
  [[nodiscard]] QUrl writeUrl() const;
  [[nodiscard]] qint64 epochNs(const DataModel::DataBlock& block, qsizetype index) const;

private:
  SinkConfig m_config;
  LineBatch m_batch;
  QNetworkAccessManager* m_manager;
  QPointer<QNetworkReply> m_reply;
  QUrl m_writeUrl;
  QString m_sslFailure;
  QByteArray m_authHeader;
  QByteArray m_postBuffer;
  qsizetype m_inFlightPoints;
  qint64 m_epochOffsetNs;
  bool m_healthy;

  std::map<int, RenderTemplate> m_templates;

  std::atomic<quint64>* m_pointsWritten;
  std::atomic<quint64>* m_pointsDropped;
  std::atomic<quint64>* m_fieldsSkipped;
  std::atomic<quint64>* m_httpErrors;
};

/**
 * @brief Per-project InfluxDB 2.x sink (Pro). Configuration rides in the project file; the API
 *        token never does, it lives in the machine-bound credential vault under scope "influxdb".
 */
class Export : public DataModel::FrameConsumer<DataModel::DataBlockPtr> {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool exportEnabled
             READ exportEnabled
             WRITE setExportEnabled
             NOTIFY enabledChanged)
  Q_PROPERTY(bool isOpen
             READ isOpen
             NOTIFY openChanged)
  Q_PROPERTY(bool hasToken
             READ hasToken
             NOTIFY configurationChanged)
  Q_PROPERTY(QString url
             READ url
             WRITE setUrl
             NOTIFY configurationChanged)
  Q_PROPERTY(QString organization
             READ organization
             WRITE setOrganization
             NOTIFY configurationChanged)
  Q_PROPERTY(QString bucket
             READ bucket
             WRITE setBucket
             NOTIFY configurationChanged)
  Q_PROPERTY(QString measurement
             READ measurement
             WRITE setMeasurement
             NOTIFY configurationChanged)
  Q_PROPERTY(QString lastError
             READ lastError
             NOTIFY statsChanged)
  Q_PROPERTY(quint64 pointsWritten
             READ pointsWritten
             NOTIFY statsChanged)
  Q_PROPERTY(quint64 pointsDropped
             READ pointsDropped
             NOTIFY statsChanged)
  Q_PROPERTY(quint64 httpErrors
             READ httpErrors
             NOTIFY statsChanged)
  // clang-format on

signals:
  void openChanged();
  void statsChanged();
  void enabledChanged();
  void configurationChanged();

private:
  explicit Export();
  Export(Export&&)                 = delete;
  Export(const Export&)            = delete;
  Export& operator=(Export&&)      = delete;
  Export& operator=(const Export&) = delete;

  ~Export() override;

public:
  [[nodiscard]] static Export& instance();
  [[nodiscard]] static bool urlSchemeAllowed(const QString& url);

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool hasToken() const;
  [[nodiscard]] bool exportEnabled() const;

  [[nodiscard]] QString url() const;
  [[nodiscard]] QString organization() const;
  [[nodiscard]] QString bucket() const;
  [[nodiscard]] QString measurement() const;
  [[nodiscard]] QString lastError() const;

  [[nodiscard]] quint64 pointsWritten() const;
  [[nodiscard]] quint64 pointsDropped() const;
  [[nodiscard]] quint64 fieldsSkipped() const;
  [[nodiscard]] quint64 httpErrors() const;

  [[nodiscard]] QJsonObject toJson() const;

public slots:
  void setupExternalConnections();
  void applyProjectConfig(const QJsonObject& config);
  void resetProjectConfig();

  void setExportEnabled(const bool enabled);
  void setUrl(const QString& url);
  void setOrganization(const QString& organization);
  void setBucket(const QString& bucket);
  void setMeasurement(const QString& measurement);
  void setToken(const QString& token);

  void ingestBlock(const DataModel::DataBlockPtr& block);

protected:
  DataModel::FrameConsumerWorkerBase* createWorker() override;

private slots:
  void onWorkerOpenChanged();
  void onWorkerError(const QString& message);
  void sampleStats();

private:
  [[nodiscard]] bool licenseValid() const;
  [[nodiscard]] SinkConfig snapshotConfig() const;
  [[nodiscard]] QString vaultHost() const;
  [[nodiscard]] quint16 vaultPort() const;

  void markConfigChanged();
  void syncToWorker();
  void reloadTokenFromVault();

private:
  static constexpr std::size_t kCacheLine = 64;

  bool m_inApply;
  bool m_exportRequested;
  bool m_savingToProjectModel;
  QString m_url;
  QString m_organization;
  QString m_bucket;
  QString m_measurement;
  QString m_token;
  QString m_lastError;

  MQTT::CredentialVault m_vault;

  alignas(kCacheLine) std::atomic<bool> m_isOpen;
  alignas(kCacheLine) std::atomic<bool> m_exportEnabled;
  alignas(kCacheLine) std::atomic<quint64> m_pointsWritten;
  alignas(kCacheLine) std::atomic<quint64> m_pointsDropped;
  alignas(kCacheLine) std::atomic<quint64> m_fieldsSkipped;
  alignas(kCacheLine) std::atomic<quint64> m_httpErrors;

  quint64 m_lastPointsWrittenSeen;
  quint64 m_lastPointsDroppedSeen;
  quint64 m_lastHttpErrorsSeen;
};

}  // namespace InfluxDB

Q_DECLARE_METATYPE(InfluxDB::SinkConfig)

#endif  // BUILD_COMMERCIAL

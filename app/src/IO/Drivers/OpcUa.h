/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <chrono>
#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QOpcUaClient>
#include <QOpcUaEndpointDescription>
#include <QOpcUaNode>
#include <QOpcUaProvider>
#include <QOpcUaReadResult>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariant>

#include "DataModel/Frame.h"
#include "IO/Drivers/OpcUaWire.h"
#include "IO/HAL_Driver.h"
#include "MQTT/CredentialVault.h"

namespace IO {
namespace Drivers {

class OpcUaTagModel;
}  // namespace Drivers
}  // namespace IO

namespace DataModel {
class ProjectModel;
}  // namespace DataModel

namespace IO {
namespace Drivers {

/**
 * @brief One subscribed variable node and how it maps onto the wire/dataset layout.
 */
struct OpcUaTag {
  QString nodeId;
  QString name;
  QString path;
  QString unit;
  OpcUaWire::Type type;
  int arrayLen;
  double min;
  double max;

  OpcUaTag() : type(OpcUaWire::Type::Invalid), arrayLen(1), min(0), max(0) {}

  [[nodiscard]] bool operator==(const OpcUaTag& other) const noexcept
  {
    return nodeId == other.nodeId && name == other.name && path == other.path && unit == other.unit
        && type == other.type && arrayLen == other.arrayLen
        && qFuzzyCompare(min + 1.0, other.min + 1.0) && qFuzzyCompare(max + 1.0, other.max + 1.0);
  }
};

/**
 * @brief HAL driver for OPC UA servers (spec 0066): endpoint discovery, anonymous or
 *        username/password authentication over a policy-None channel, monitored-item
 *        subscription with a timed-read fallback, and one delta frame per publishing tick.
 */
class OpcUa : public HAL_Driver {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString endpointUrl
             READ endpointUrl
             WRITE setEndpointUrl
             NOTIFY endpointUrlChanged)
  Q_PROPERTY(int endpointIndex
             READ endpointIndex
             WRITE setEndpointIndex
             NOTIFY endpointIndexChanged)
  Q_PROPERTY(QStringList endpointList
             READ endpointList
             NOTIFY endpointsChanged)
  Q_PROPERTY(QVariantList endpointSelectable
             READ endpointSelectable
             NOTIFY endpointsChanged)
  Q_PROPERTY(bool discovering
             READ discovering
             NOTIFY discoveringChanged)
  Q_PROPERTY(int authMode
             READ authMode
             WRITE setAuthMode
             NOTIFY authModeChanged)
  Q_PROPERTY(QString username
             READ username
             WRITE setUsername
             NOTIFY usernameChanged)
  Q_PROPERTY(QString password
             READ password
             WRITE setPassword
             NOTIFY passwordChanged)
  Q_PROPERTY(int publishingInterval
             READ publishingInterval
             WRITE setPublishingInterval
             NOTIFY publishingIntervalChanged)
  Q_PROPERTY(int tagCount
             READ tagCount
             NOTIFY tagsChanged)
  Q_PROPERTY(QString statusText
             READ statusText
             NOTIFY statusChanged)
  Q_PROPERTY(bool pollMode
             READ pollMode
             NOTIFY statusChanged)
  Q_PROPERTY(int revisedInterval
             READ revisedInterval
             NOTIFY statusChanged)
  Q_PROPERTY(bool browsing
             READ browsing
             NOTIFY browsingChanged)
  Q_PROPERTY(QStringList authModeList
             READ authModeList
             NOTIFY languageChanged)
  Q_PROPERTY(QObject* tagModel
             READ tagModelObject
             CONSTANT)
  // clang-format on

signals:
  void tagsChanged();
  void statusChanged();
  void authModeChanged();
  void usernameChanged();
  void passwordChanged();
  void browsingChanged();
  void languageChanged();
  void endpointsChanged();
  void endpointUrlChanged();
  void discoveringChanged();
  void endpointIndexChanged();
  void publishingIntervalChanged();
  void browseFailed(const QString& reason);

public:
  explicit OpcUa();
  ~OpcUa();

  OpcUa(OpcUa&&)                 = delete;
  OpcUa(const OpcUa&)            = delete;
  OpcUa& operator=(OpcUa&&)      = delete;
  OpcUa& operator=(const OpcUa&) = delete;

  void close() override;
  void setPersistent(const bool persistent) noexcept;

  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isConnecting() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;
  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override;

  [[nodiscard]] QString endpointUrl() const;
  [[nodiscard]] int endpointIndex() const;
  [[nodiscard]] QStringList endpointList() const;
  [[nodiscard]] QVariantList endpointSelectable() const;
  [[nodiscard]] QJsonArray endpointsJson() const;
  [[nodiscard]] bool tagsDeferred() const;
  [[nodiscard]] bool discovering() const;
  [[nodiscard]] int authMode() const;
  [[nodiscard]] QString username() const;
  [[nodiscard]] QString password() const;
  [[nodiscard]] int publishingInterval() const;
  [[nodiscard]] int tagCount() const;
  [[nodiscard]] QString statusText() const;
  [[nodiscard]] bool pollMode() const;
  [[nodiscard]] int revisedInterval() const;
  [[nodiscard]] bool browsing() const;
  [[nodiscard]] QStringList authModeList() const;

  [[nodiscard]] const QList<OpcUaTag>& tags() const noexcept;
  [[nodiscard]] QJsonArray tagsJson() const;
  [[nodiscard]] QJsonArray wireSchema() const;
  [[nodiscard]] QJsonObject statusJson() const;
  [[nodiscard]] QStringList badTags() const;
  [[nodiscard]] QJsonObject buildProject() const;
  [[nodiscard]] DataModel::ProjectModel* loadGeneratedProject();
  [[nodiscard]] OpcUaTagModel* tagModel();
  [[nodiscard]] QObject* tagModelObject();

  [[nodiscard]] Q_INVOKABLE QString tagInfo(const int index) const;

public slots:
  void discoverEndpoints();
  void startBrowse();
  void stopBrowse();
  void cancelBrowse();
  void generateProject();
  void setupExternalConnections();
  void setDriverProperty(const QString& key, const QVariant& value) override;
  void setEndpointUrl(const QString& url);
  void setEndpointIndex(const int index);
  void setAuthMode(const int mode);
  void setUsername(const QString& username);
  void setPassword(const QString& password);
  void setPublishingInterval(const int interval);
  void setTags(const QJsonArray& tags);
  void addTag(const OpcUaTag& tag);
  void removeTag(const int index);
  void clearTags();

private slots:
  void onStateChanged(QOpcUaClient::ClientState state);
  void onErrorChanged(QOpcUaClient::ClientError error);
  void onEndpointsFinished(const QList<QOpcUaEndpointDescription>& endpoints,
                           QOpcUa::UaStatusCode status);
  void onBrowseClientState(QOpcUaClient::ClientState state);
  void onMonitoringEnabled(int tag, QOpcUa::UaStatusCode status);
  void onValueUpdated(int tag, const QVariant& value);
  void onReadFinished(const QList<QOpcUaReadResult>& results, QOpcUa::UaStatusCode status);
  void onDialTimeout();
  void onPollTick();
  void onFrameTick();
  void onWatchdogTick();

private:
  void doClose();
  void teardownClient(QOpcUaClient*& client);
  void failDial(const QString& reason);
  void onLinkDropped(const QString& reason);
  void applyAuthentication(QOpcUaClient* client) const;
  [[nodiscard]] QOpcUaClient* makeClient();
  [[nodiscard]] static QOpcUaProvider& provider();
  [[nodiscard]] QString selectedEndpointUrl() const;
  [[nodiscard]] bool hasSelectedEndpoint() const noexcept;
  void continuePendingDial();
  [[nodiscard]] static bool policyIsNone(const QOpcUaEndpointDescription& endpoint);

  void subscribeAll();
  void enterPollMode(const QString& reason);
  void adoptRevisedInterval(int tag);
  void issueRead(const QList<int>& tags);
  [[nodiscard]] QOpcUaEndpointDescription dialEndpoint() const;
  [[nodiscard]] static bool endpointAcceptsToken(const QOpcUaEndpointDescription& endpoint,
                                                 const int authMode);
  void markBad(int tag);
  [[nodiscard]] static QVariant unwrapValue(const QVariant& value);
  void storeValue(int tag,
                  const QVariant& value,
                  QOpcUa::UaStatusCode status,
                  const QDateTime& sourceTs);
  void reserveFrame();
  void readServerLimits();
  [[nodiscard]] bool tagsFrozen() const;
  void applyDeferredTags();
  [[nodiscard]] CapturedData::SteadyTimePoint toSteady(const QDateTime& sourceTs);
  [[nodiscard]] static OpcUaWire::Type wireTypeFor(const OpcUaTag& tag) noexcept;

  [[nodiscard]] static DataModel::Dataset datasetFor(const OpcUaTag& tag, int element, int index);
  [[nodiscard]] static OpcUaTag tagFromJson(const QJsonObject& obj);
  [[nodiscard]] static QJsonObject tagToJson(const OpcUaTag& tag);
  void loadSettings();
  void saveTags();

  /**
   * @brief Which session is waiting for endpoint discovery before it can dial.
   */
  enum class PendingDial : quint8 {
    None,
    Live,
    Browse,
  };

  /**
   * @brief Per-wire-index cache slot: the newest value, its status and source time.
   */
  struct Slot {
    QVariant value;
    QDateTime sourceTs;
    OpcUaWire::Type type;
    bool dirty;
    bool bad;
    bool warned;

    Slot() : type(OpcUaWire::Type::Str), dirty(false), bad(false), warned(false) {}
  };

  bool m_connecting;
  bool m_discovering;
  bool m_browsing;
  bool m_pollMode;
  bool m_persistent;
  bool m_readInFlight;
  bool m_subscribing;
  bool m_hasDeferred;
  PendingDial m_pendingDial;
  bool m_clockValid;
  int m_authMode;
  int m_endpointIndex;
  int m_publishingInterval;
  int m_pendingMonitors;
  int m_failedMonitors;
  int m_revisedInterval;
  int m_frameCursor;
  int m_readLimit;
  quint64 m_valuesReceived;
  quint64 m_badStatusCount;
  quint64 m_unstampedCount;
  quint64 m_framesPublished;
  quint64 m_linkDrops;
  quint64 m_skippedPolls;
  QString m_username;
  QString m_password;
  QString m_endpointUrl;
  QString m_lastError;
  QByteArray m_frame;
  QTimer* m_dialTimer;
  QTimer* m_watchdog;
  QTimer* m_pollTimer;
  QTimer* m_frameTimer;
  QOpcUaClient* m_client;
  QOpcUaClient* m_browseClient;
  QOpcUaClient* m_discoveryClient;
  OpcUaTagModel* m_tagModel;
  QList<Slot> m_slots;
  QList<int> m_firstIndex;
  QList<int> m_slotCount;
  QList<int> m_polledTags;
  qsizetype m_frameBytes;
  qint64 m_lastStampNs;
  qint64 m_lastNotifyNs;
  qint64 m_serverOffsetMs;
  QList<OpcUaTag> m_tags;
  QJsonArray m_deferredTags;
  QList<QOpcUaNode*> m_nodes;
  QHash<QString, int> m_nodeIndex;
  QList<QOpcUaEndpointDescription> m_endpoints;
  qint64 m_clockOffsetNs;
  ::MQTT::CredentialVault m_vault;
  QSettings m_settings;
};

}  // namespace Drivers
}  // namespace IO

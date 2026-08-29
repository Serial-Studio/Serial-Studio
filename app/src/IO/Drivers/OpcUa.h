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
#include <QPointer>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariant>

#include "DataModel/Frame.h"
#include "IO/Drivers/OpcUa/OpcUaBrowser.h"
#include "IO/Drivers/OpcUa/OpcUaCertificateStore.h"
#include "IO/Drivers/OpcUa/OpcUaFrameAssembler.h"
#include "IO/Drivers/OpcUa/OpcUaTag.h"
#include "IO/Drivers/OpcUaSession.h"
#include "IO/Drivers/OpcUaTypes.h"
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
 * @brief HAL driver for OPC UA servers (spec 0066): endpoint discovery, anonymous or
 *        username/password authentication, monitored-item subscription with a timed-read fallback,
 *        and one delta frame per publishing tick. The certificate store, browse session and value
 *        cache are sub-objects (spec 0070); this class stays the session, dial and config core.
 */
class OpcUa
  : public HAL_Driver
  , public OpcUaBrowseHost {
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
  Q_PROPERTY(QString securityPolicy
             READ securityPolicy
             WRITE setSecurityPolicy
             NOTIFY securityChanged)
  Q_PROPERTY(int securityPolicyIndex
             READ securityPolicyIndex
             WRITE setSecurityPolicyIndex
             NOTIFY securityChanged)
  Q_PROPERTY(QStringList securityPolicyList
             READ securityPolicyList
             NOTIFY languageChanged)
  Q_PROPERTY(QVariantList securityPolicyDeprecated
             READ securityPolicyDeprecated
             CONSTANT)
  Q_PROPERTY(int securityMode
             READ securityMode
             WRITE setSecurityMode
             NOTIFY securityChanged)
  Q_PROPERTY(QStringList securityModeList
             READ securityModeList
             NOTIFY languageChanged)
  Q_PROPERTY(QString userCertificatePath
             READ userCertificatePath
             WRITE setUserCertificatePath
             NOTIFY securityChanged)
  Q_PROPERTY(QString userKeyPath
             READ userKeyPath
             WRITE setUserKeyPath
             NOTIFY securityChanged)
  Q_PROPERTY(QVariantMap clientCertificate
             READ clientCertificate
             NOTIFY certificateChanged)
  Q_PROPERTY(QVariantList trustedCertificates
             READ trustedCertificates
             NOTIFY certificateChanged)
  Q_PROPERTY(bool credentialsExposed
             READ credentialsExposed
             NOTIFY securityChanged)
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
  void securityChanged();
  void certificateChanged();
  void serverCertificateUntrusted(const QVariantMap& certificate, const QString& reason);

public:
  explicit OpcUa();
  ~OpcUa();

  OpcUa(OpcUa&&)                 = delete;
  OpcUa(const OpcUa&)            = delete;
  OpcUa& operator=(OpcUa&&)      = delete;
  OpcUa& operator=(const OpcUa&) = delete;

  void close() override;
  void setPersistent(const bool persistent) noexcept;
  void setSessionPeer(OpcUa* peer);

  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isConnecting() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;
  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override;

  [[nodiscard]] QString endpointUrl() const override;
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

  [[nodiscard]] QString securityPolicy() const;
  [[nodiscard]] int securityPolicyIndex() const;
  [[nodiscard]] QStringList securityPolicyList() const;
  [[nodiscard]] static const QStringList& supportedPolicies();
  [[nodiscard]] QVariantList securityPolicyDeprecated() const;
  [[nodiscard]] int securityMode() const;
  [[nodiscard]] QStringList securityModeList() const;
  [[nodiscard]] QString userCertificatePath() const;
  [[nodiscard]] QString userKeyPath() const;
  [[nodiscard]] QVariantMap clientCertificate() const;
  [[nodiscard]] QVariantList trustedCertificates() const;
  [[nodiscard]] bool credentialsExposed() const;
  [[nodiscard]] QJsonObject certificateJson() const;
  [[nodiscard]] QString negotiatedPolicy() const;
  [[nodiscard]] int negotiatedMode() const;
  [[nodiscard]] QJsonArray trustedJson() const;

  [[nodiscard]] const QList<OpcUaTag>& tags() const noexcept override;
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
  void setSecurityPolicy(const QString& policyUri);
  void setSecurityPolicyIndex(const int index);
  void setSecurityMode(const int mode);
  void setUserCertificatePath(const QString& value);
  void setUserKeyPath(const QString& value);
  [[nodiscard]] bool regenerateCertificate();
  [[nodiscard]] bool exportCertificate(const QString& path);
  [[nodiscard]] bool trustServerCertificate(const QString& fingerprint);
  [[nodiscard]] bool revokeServerCertificate(const QString& fingerprint);
  void setTags(const QJsonArray& tags);
  void addTag(const OpcUaTag& tag);
  void removeTag(const int index);
  void clearTags();

private slots:
  void onSessionConnected();
  void onSessionDisconnected();
  void onConnectFailed(const QString& reason);
  void onEndpointsFinished(const QList<OpcUaTypes::Endpoint>& endpoints,
                           OpcUaTypes::StatusCode status);
  void onBrowseFailed(const QString& reason);
  void onTypeMismatch(int index, const QString& declared, const QString& actual);
  void onSubscribed(const QList<OpcUaTypes::StatusCode>& perItemStatus);
  void onSubscriptionLost(const QString& reason);
  void onValueChanged(const OpcUaTypes::MonitoredValue& value);
  void onReadFinished(quint32 token,
                      const QList<OpcUaTypes::ReadRow>& rows,
                      OpcUaTypes::StatusCode status);
  void onDialTimeout();
  void onDiscoveryTimeout();
  void onPollTick();
  void onFrameTick();
  void onWatchdogTick();

private:
  void doClose();
  void teardownSession(OpcUaSession*& session);
  void failDial(const QString& reason);
  void onLinkDropped(const QString& reason);
  void continuePendingDial();
  void startDial();
  void selectBestEndpoint(const QString& previousUrl);
  void publishEndpointSelection(const int index);
  void warnAboutPlaintextCredentials() const;
  [[nodiscard]] bool credentialsAreExposed() const;
  [[nodiscard]] static QString describeMode(OpcUaTypes::SecurityMode mode);

  void prepareClientIdentity() override;
  void requestBrowseDiscovery() override;
  void commitBrowsedTags(const QJsonArray& tags) override;
  void reportTrustFailure(const OpcUaSession* session) override;
  [[nodiscard]] OpcUaSession* makeSession() override;
  [[nodiscard]] bool hasSelectedEndpoint() const noexcept override;
  [[nodiscard]] QString selectedEndpointUrl() const override;
  [[nodiscard]] OpcUaTypes::Endpoint dialEndpoint() const override;
  [[nodiscard]] OpcUaSession::Identity identity() const override;

  void subscribeAll();
  void enterPollMode(const QString& reason);
  void adoptRevisedInterval();
  void issueRead(const QList<int>& tags);
  [[nodiscard]] bool tagsFrozen() const;
  [[nodiscard]] const OpcUa* sessionPeer() const;
  void applyDeferredTags();

  [[nodiscard]] static DataModel::Dataset datasetFor(const OpcUaTag& tag, int element, int index);
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

  bool m_connecting;
  bool m_discovering;
  bool m_pollMode;
  bool m_persistent;
  bool m_readInFlight;
  bool m_subscribing;
  bool m_hasDeferred;
  PendingDial m_pendingDial;
  int m_authMode;
  int m_endpointIndex;
  int m_publishingInterval;
  int m_pendingMonitors;
  int m_failedMonitors;
  int m_revisedInterval;
  quint64 m_framesPublished;
  quint64 m_linkDrops;
  quint64 m_skippedPolls;
  QString m_username;
  QString m_password;
  QString m_endpointUrl;
  QString m_lastError;
  QString m_securityPolicy;
  QString m_userCertificatePath;
  QString m_userKeyPath;
  int m_securityMode;
  QTimer* m_dialTimer;
  QTimer* m_discoveryTimer;
  QTimer* m_watchdog;
  QTimer* m_pollTimer;
  QTimer* m_frameTimer;
  OpcUaSession* m_session;
  OpcUaSession* m_discoverySession;
  QPointer<OpcUa> m_sessionPeer;
  QList<int> m_polledTags;
  qint64 m_lastNotifyNs;
  QList<OpcUaTag> m_tags;
  QJsonArray m_deferredTags;
  QHash<QString, int> m_nodeIndex;
  QList<OpcUaTypes::Endpoint> m_endpoints;
  OpcUaCertificateStore m_certificates;
  OpcUaFrameAssembler m_assembler;
  OpcUaBrowser m_browser;
  ::MQTT::CredentialVault m_vault;
  QSettings m_settings;
};

}  // namespace Drivers
}  // namespace IO

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
#include <utility>

#include "DataModel/Frame.h"
#include "IO/Drivers/OpcUa/OpcUaBrowser.h"
#include "IO/Drivers/OpcUa/OpcUaCertificateStore.h"
#include "IO/Drivers/OpcUa/OpcUaEndpointSelection.h"
#include "IO/Drivers/OpcUa/OpcUaSubscriptions.h"
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
 *        and one delta frame per publishing tick. Certificate store, browse session, subscriptions
 *        and project builder are sub-objects (spec 0070); this class alone owns the open verdict.
 */
class OpcUa
  : public HAL_Driver
  , public OpcUaBrowseHost
  , public OpcUaSubscriptionHost {
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

  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;
  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override;

  [[nodiscard]] bool isReadable() const noexcept override { return isOpen(); }

  [[nodiscard]] bool isWritable() const noexcept override { return false; }

  [[nodiscard]] bool isConnecting() const noexcept override { return m_connecting; }

  [[nodiscard]] bool isOpen() const noexcept override { return m_session && m_session->isOpen(); }

  [[nodiscard]] qint64 write(const QByteArray& data) override
  {
    Q_UNUSED(data)
    return 0;
  }

  [[nodiscard]] QJsonArray endpointsJson() const;
  [[nodiscard]] QString statusText() const;
  [[nodiscard]] bool pollMode() const;
  [[nodiscard]] int revisedInterval() const;
  [[nodiscard]] QStringList authModeList() const;

  [[nodiscard]] int authMode() const { return m_authMode; }

  [[nodiscard]] bool discovering() const { return m_discovering; }

  [[nodiscard]] bool tagsDeferred() const { return m_hasDeferred; }

  [[nodiscard]] QString username() const { return m_username; }

  [[nodiscard]] QString password() const { return m_password; }

  [[nodiscard]] int endpointIndex() const { return m_endpointIndex; }

  [[nodiscard]] bool browsing() const { return m_browser.browsing(); }

  [[nodiscard]] QString endpointUrl() const override { return m_endpointUrl; }

  [[nodiscard]] int tagCount() const { return static_cast<int>(m_tags.size()); }

  [[nodiscard]] int publishingInterval() const override { return m_publishingInterval; }

  [[nodiscard]] int securityPolicyIndex() const;
  [[nodiscard]] static const QStringList& supportedPolicies();
  [[nodiscard]] QVariantMap clientCertificate() const;
  [[nodiscard]] QVariantList trustedCertificates() const;
  [[nodiscard]] QString negotiatedPolicy() const;
  [[nodiscard]] int negotiatedMode() const;

  [[nodiscard]] QStringList securityModeList() const
  {
    return OpcUaEndpointSelection::securityModeNames();
  }

  [[nodiscard]] QStringList securityPolicyList() const
  {
    return OpcUaEndpointSelection::policyNames();
  }

  [[nodiscard]] QVariantList securityPolicyDeprecated() const
  {
    return OpcUaEndpointSelection::policyDeprecationFlags();
  }

  [[nodiscard]] QStringList endpointList() const
  {
    return OpcUaEndpointSelection::endpointRows(m_endpoints);
  }

  [[nodiscard]] QVariantList endpointSelectable() const
  {
    return OpcUaEndpointSelection::endpointSelectable(m_endpoints, m_authMode);
  }

  [[nodiscard]] int securityMode() const { return m_securityMode; }

  [[nodiscard]] QString securityPolicy() const { return m_securityPolicy; }

  [[nodiscard]] QString userKeyPath() const { return m_userKeyPath; }

  [[nodiscard]] QString userCertificatePath() const { return m_userCertificatePath; }

  [[nodiscard]] bool credentialsExposed() const { return credentialsAreExposed(); }

  [[nodiscard]] QJsonArray trustedJson() const { return m_certificates.trustedJson(); }

  [[nodiscard]] QJsonObject certificateJson() const { return m_certificates.certificateJson(); }

  [[nodiscard]] QJsonArray tagsJson() const;
  [[nodiscard]] QJsonArray wireSchema() const;
  [[nodiscard]] QJsonObject statusJson() const;
  [[nodiscard]] QJsonObject buildProject() const;
  [[nodiscard]] DataModel::ProjectModel* loadGeneratedProject();

  [[nodiscard]] OpcUaTagModel* tagModel() { return m_browser.tagModel(); }

  [[nodiscard]] QObject* tagModelObject() { return m_browser.tagModelObject(); }

  [[nodiscard]] QStringList badTags() const { return m_subscriptions.badTags(); }

  [[nodiscard]] const QList<OpcUaTag>& tags() const noexcept override { return m_tags; }

  [[nodiscard]] Q_INVOKABLE QString tagInfo(const int index) const;

public slots:
  void discoverEndpoints();

  void startBrowse() { m_browser.start(); }

  void stopBrowse() { m_browser.stop(); }

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
  void onDialTimeout();
  void onDiscoveryTimeout();

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

  void prepareClientIdentity() override;
  void requestBrowseDiscovery() override;
  void reportTrustFailure(const OpcUaSession* session) override;

  void commitBrowsedTags(const QJsonArray& tags) override { setTags(tags); }

  [[nodiscard]] OpcUaSession* makeSession() override { return new OpcUaSession(this); }

  [[nodiscard]] bool hasSelectedEndpoint() const noexcept override;
  [[nodiscard]] QString selectedEndpointUrl() const override;
  [[nodiscard]] OpcUaTypes::Endpoint dialEndpoint() const override;
  [[nodiscard]] OpcUaSession::Identity identity() const override;

  [[nodiscard]] OpcUaSession* liveSession() const override { return m_session; }

  void publishFrame(QByteArray&& frame, CapturedData::SteadyTimePoint timestamp) override
  {
    publishReceivedData(std::move(frame), timestamp);
  }

  void reportDriverError(const QString& title, const QString& detail) const override
  {
    logDriverError(title, detail);
  }

  [[nodiscard]] bool tagsFrozen() const;
  [[nodiscard]] const OpcUa* sessionPeer() const;
  void applyDeferredTags();

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
  bool m_persistent;
  bool m_hasDeferred;
  PendingDial m_pendingDial;
  int m_authMode;
  int m_endpointIndex;
  int m_publishingInterval;
  quint64 m_linkDrops;
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
  OpcUaSession* m_session;
  OpcUaSession* m_discoverySession;
  QPointer<OpcUa> m_sessionPeer;
  QList<OpcUaTag> m_tags;
  QJsonArray m_deferredTags;
  QList<OpcUaTypes::Endpoint> m_endpoints;
  OpcUaCertificateStore m_certificates;
  OpcUaSubscriptions m_subscriptions;
  OpcUaBrowser m_browser;
  ::MQTT::CredentialVault m_vault;
  QSettings m_settings;
};

}  // namespace Drivers
}  // namespace IO

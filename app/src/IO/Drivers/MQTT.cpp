/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#include "IO/Drivers/MQTT.h"

#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QLoggingCategory>
#include <QRandomGenerator>
#include <QStandardPaths>

#include "Licensing/CommercialToken.h"
#include "Misc/Utilities.h"
#include "SSAssert.h"

Q_LOGGING_CATEGORY(lcMqttSub, "serialstudio.mqtt.subscriber", QtCriticalMsg)

//--------------------------------------------------------------------------------------------------
// Constants: per-step deadlines. Attempts and backoff belong to the shared Async::RetryPolicy.
//--------------------------------------------------------------------------------------------------

static constexpr int kBrokerConnectTimeoutMs    = 15000;
static constexpr int kBrokerDisconnectTimeoutMs = 5000;

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the MQTT input driver and restores persisted broker settings.
 */
IO::Drivers::MQTT::MQTT()
  : m_sslEnabled(false)
  , m_cleanSession(true)
  , m_autoKeepAlive(true)
  , m_userWantsOpen(false)
  , m_connecting(false)
  , m_failureNotified(false)
  , m_sessionEstablished(false)
  , m_port(1883)
  , m_keepAlive(60)
  , m_protocolVersion(QMqttClient::MQTT_5_0)
  , m_hostname(QStringLiteral("127.0.0.1"))
  , m_alpnEnabled(false)
  , m_alpnProtocol(QStringLiteral("x-amzn-mqtt-ca"))
  , m_runner(this)
{
  m_mqttVersions.insert(tr("MQTT 3.1"), QMqttClient::MQTT_3_1);
  m_mqttVersions.insert(tr("MQTT 3.1.1"), QMqttClient::MQTT_3_1_1);
  m_mqttVersions.insert(tr("MQTT 5.0"), QMqttClient::MQTT_5_0);

  m_sslProtocols.insert(tr("TLS 1.2"), QSsl::TlsV1_2);
  m_sslProtocols.insert(tr("TLS 1.3"), QSsl::TlsV1_3);
  m_sslProtocols.insert(tr("TLS 1.3 or Later"), QSsl::TlsV1_3OrLater);
  m_sslProtocols.insert(tr("DTLS 1.2 or Later"), QSsl::DtlsV1_2OrLater);
  m_sslProtocols.insert(tr("Any Protocol"), QSsl::AnyProtocol);
  m_sslProtocols.insert(tr("Secure Protocols Only"), QSsl::SecureProtocols);

  m_peerVerifyModes.insert(tr("None"), QSslSocket::VerifyNone);
  m_peerVerifyModes.insert(tr("Query Peer"), QSslSocket::QueryPeer);
  m_peerVerifyModes.insert(tr("Verify Peer"), QSslSocket::VerifyPeer);
  m_peerVerifyModes.insert(tr("Auto Verify Peer"), QSslSocket::AutoVerifyPeer);

  m_sslConfiguration.setProtocol(QSsl::SecureProtocols);
  m_sslConfiguration.setPeerVerifyMode(QSslSocket::AutoVerifyPeer);
  m_sslConfiguration.setPeerVerifyDepth(10);

  connect(&m_client, &QMqttClient::stateChanged, this, &MQTT::onStateChanged);
  connect(&m_client, &QMqttClient::errorChanged, this, &MQTT::onErrorChanged);
  connect(&m_client, &QMqttClient::messageReceived, this, &MQTT::onMessageReceived);
  connect(&m_runner, &Async::TaskRunner::finished, this, &MQTT::onOpenFlowFinished);

  loadPersistedSettings();
  if (m_clientId.isEmpty())
    regenerateClientId();

  applyPendingToClient();

  connect(this, &MQTT::mqttConfigurationChanged, this, &MQTT::configurationChanged);
  connect(this, &MQTT::sslConfigurationChanged, this, &MQTT::configurationChanged);
  connect(this, &MQTT::connectedChanged, this, &MQTT::configurationChanged);

  Q_EMIT configurationChanged();
}

/**
 * @brief Destructor; closes any active broker connection.
 */
IO::Drivers::MQTT::~MQTT()
{
  close();
}

//--------------------------------------------------------------------------------------------------
// HAL driver implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Disconnects from the MQTT broker. The flow is cancelled first so a pending attempt
 *        cannot re-open the session behind the close.
 */
void IO::Drivers::MQTT::close()
{
  m_userWantsOpen      = false;
  m_connecting         = false;
  m_failureNotified    = false;
  m_sessionEstablished = false;
  m_runner.cancel();

  if (m_client.state() != QMqttClient::Disconnected)
    m_client.disconnectFromHost();
}

/**
 * @brief Returns true when the broker connection is established.
 */
bool IO::Drivers::MQTT::isOpen() const noexcept
{
  return m_client.state() == QMqttClient::Connected;
}

/**
 * @brief Returns true when the driver can receive payloads (subscriber-only).
 */
bool IO::Drivers::MQTT::isReadable() const noexcept
{
  return isOpen();
}

/**
 * @brief Returns false; MQTT input driver does not transmit.
 */
bool IO::Drivers::MQTT::isWritable() const noexcept
{
  return false;
}

/**
 * @brief Configuration is OK when host, port and topic filter are set.
 */
bool IO::Drivers::MQTT::configurationOk() const noexcept
{
  return !m_hostname.isEmpty() && m_port > 0 && !m_topicFilter.isEmpty();
}

/**
 * @brief Subscriber-only driver: writes are rejected.
 */
qint64 IO::Drivers::MQTT::write(const QByteArray& data)
{
  Q_UNUSED(data);
  return -1;
}

/**
 * @brief Returns true; the driver opens through an orchestrated, non-blocking flow.
 */
bool IO::Drivers::MQTT::supportsAsyncOpen() const noexcept
{
  return true;
}

/**
 * @brief Starts an open attempt and reports whether the request was accepted, which is what this
 *        driver always reported: the broker session itself has never been established on return.
 */
bool IO::Drivers::MQTT::open(const QIODevice::OpenMode mode)
{
  beginOpen(mode);
  return m_userWantsOpen;
}

/**
 * @brief Starts the broker session: dial, wait, subscribe (a rejected filter fails the open).
 *        The notification latch clears only on a request's first attempt: retries re-enter with
 *        m_userWantsOpen already set, keeping the error box at one per open request.
 */
void IO::Drivers::MQTT::beginOpen(const QIODevice::OpenMode mode)
{
  Q_UNUSED(mode);

  if (!m_userWantsOpen)
    m_failureNotified = false;

  QString reason;
  if (!openRequestAccepted(reason)) {
    Q_EMIT openFinished(false, reason);
    return;
  }

  if (m_client.state() == QMqttClient::Connected) {
    qCInfo(lcMqttSub) << "open() no-op; state already" << m_client.state();
    m_userWantsOpen = true;
    Q_EMIT openFinished(true, QString());
    return;
  }

  if (m_clientId.isEmpty())
    regenerateClientId();

  m_connecting    = true;
  m_userWantsOpen = true;
  m_runner.run(buildOpenFlow());
}

/**
 * @brief Latches the driver's one failure box and answers whether the caller may show it: the
 *        driver boxes only a failure of a session that was never up (license, configuration, a
 *        refused first connect), verbatim; once a session existed the give-up is reported by the
 *        connection manager as a lost link, so a box here would be the second for one event.
 */
bool IO::Drivers::MQTT::claimFailureReport()
{
  if (m_failureNotified || m_sessionEstablished)
    return false;

  m_failureNotified = true;
  return true;
}

/**
 * @brief Validates the open request, reporting the license warning at most once per request so a
 *        retried attempt cannot stack message boxes.
 */
bool IO::Drivers::MQTT::openRequestAccepted(QString& reason)
{
  const auto& token = Licensing::CommercialToken::current();
  if (!token.isValid() || !SS_LICENSE_GUARD()) {
    if (claimFailureReport())
      Misc::Utilities::showMessageBox(
        tr("MQTT Feature Requires a Commercial License"),
        tr("Subscribing to an MQTT broker is only available with a valid Serial Studio license "
           "or an active trial."),
        QMessageBox::Warning);

    reason = QStringLiteral("a commercial license is required");
    return false;
  }

  if (m_hostname.isEmpty() || m_port == 0) {
    qCWarning(lcMqttSub) << "open() rejected: missing hostname or port" << m_hostname << m_port;
    reason = QStringLiteral("the broker hostname or port is missing");
    return false;
  }

  if (m_topicFilter.isEmpty()) {
    qCWarning(lcMqttSub) << "open() rejected: empty topic filter";
    reason = QStringLiteral("the topic filter is empty");
    return false;
  }

  return true;
}

/**
 * @brief Composes the open sequence against the client's current state, so an attempt that finds
 *        a session already on its way waits for it instead of dialing a second one.
 */
Async::Task* IO::Drivers::MQTT::buildOpenFlow()
{
  auto* group = Async::sequential(QStringLiteral("mqtt-open"));

  if (m_client.state() == QMqttClient::Disconnected)
    group->addChild(Async::invoke(QStringLiteral("broker-dial"),
                                  [this](QString& reason) { return dialBroker(reason); }));

  group->addChild(makeConnectStep());
  group->addChild(Async::invoke(QStringLiteral("broker-subscribe"),
                                [this](QString& reason) { return subscribeToTopic(reason); }));

  return group;
}

/**
 * @brief Composes the reconnect a broker-setting change needs. The disconnect is requested before
 *        composing so the wait step is only added when the session is genuinely still up.
 */
Async::Task* IO::Drivers::MQTT::buildReconnectFlow()
{
  auto* group = Async::sequential(QStringLiteral("mqtt-reconnect"));

  if (m_client.state() != QMqttClient::Disconnected) {
    auto* wait = Async::awaitSignal(QStringLiteral("broker-disconnect"));
    wait->onSuccess(&m_client, &QMqttClient::disconnected);
    group->addChild(Async::timeout(wait, kBrokerDisconnectTimeoutMs, m_runner.clock()));
  }

  if (!m_userWantsOpen)
    return group;

  group->addChild(Async::invoke(QStringLiteral("broker-dial"),
                                [this](QString& reason) { return dialBroker(reason); }));
  group->addChild(makeConnectStep());
  group->addChild(Async::invoke(QStringLiteral("broker-subscribe"),
                                [this](QString& reason) { return subscribeToTopic(reason); }));

  return group;
}

/**
 * @brief Builds the bounded wait for the broker to accept the session.
 */
Async::Task* IO::Drivers::MQTT::makeConnectStep()
{
  auto* step = Async::awaitSignal(QStringLiteral("broker-connect"));
  step->onSuccess(&m_client, &QMqttClient::connected);
  step->onFailure(
    &m_client, &QMqttClient::disconnected, QStringLiteral("the broker closed the connection"));
  step->setAbortHandler([this]() { m_client.disconnectFromHost(); });

  return Async::timeout(step, kBrokerConnectTimeoutMs, m_runner.clock());
}

/**
 * @brief Pushes the staged settings into the client and dials the broker.
 */
bool IO::Drivers::MQTT::dialBroker(QString& reason)
{
  if (m_client.state() != QMqttClient::Disconnected) {
    reason = QStringLiteral("the client is already busy");
    return false;
  }

  applyPendingToClient();

  qCInfo(lcMqttSub).nospace() << "Connecting to " << (m_sslEnabled ? "mqtts://" : "mqtt://")
                              << m_hostname << ":" << m_port << " clientId=" << m_clientId
                              << " topic=" << m_topicFilter;

  if (m_sslEnabled)
    m_client.connectToHostEncrypted(m_sslConfiguration);
  else
    m_client.connectToHost();

  return true;
}

/**
 * @brief Subscribes to the configured filter, dropping the session when the broker refuses it so
 *        the next attempt starts from a clean state instead of a connected but silent source.
 */
bool IO::Drivers::MQTT::subscribeToTopic(QString& reason)
{
  if (m_topicFilter.isEmpty())
    return true;

  QMqttTopicFilter filter;
  filter.setFilter(m_topicFilter);

  auto* sub = m_client.subscribe(filter, 0);
  if (!sub || sub->state() == QMqttSubscription::Error) {
    qCCritical(lcMqttSub) << "subscribe failed for filter" << m_topicFilter;
    m_client.disconnectFromHost();
    reason = tr("Failed to subscribe to topic \"%1\".").arg(m_topicFilter);
    return false;
  }

  qCInfo(lcMqttSub) << "subscribed to" << m_topicFilter << "initial state:" << sub->state();
  connect(
    sub, &QMqttSubscription::stateChanged, this, [this](QMqttSubscription::SubscriptionState s) {
      qCInfo(lcMqttSub) << "subscription state for" << m_topicFilter << "->" << s;
    });

  return true;
}

/**
 * @brief Reports the attempt's outcome. A cancel is a close the owner asked for and says nothing,
 *        and neither does a flow that only tore a session down for a settings change. A flow that
 *        succeeds without a live session lost the broker between subscribe and completion, and
 *        must still report, or the owner waits on an open that will never finish.
 */
void IO::Drivers::MQTT::onOpenFlowFinished(Async::Outcome outcome, const Async::StepError& error)
{
  if (outcome == Async::Outcome::Cancelled)
    return;

  m_connecting = false;
  if (outcome == Async::Outcome::Success) {
    if (isOpen())
      Q_EMIT openFinished(true, QString());
    else if (m_userWantsOpen)
      Q_EMIT openFinished(false, tr("The broker closed the connection."));

    return;
  }

  const QString reason = error.reason.isEmpty() ? QStringLiteral("open failed") : error.reason;
  Q_EMIT openFinished(false, reason);
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether SSL/TLS is enabled for the broker connection.
 */
bool IO::Drivers::MQTT::sslEnabled() const noexcept
{
  return m_sslEnabled;
}

/**
 * @brief Returns whether Qt manages keep-alive PINGREQ automatically.
 */
bool IO::Drivers::MQTT::autoKeepAlive() const noexcept
{
  return m_autoKeepAlive;
}

/**
 * @brief Returns whether the clean session flag is set.
 */
bool IO::Drivers::MQTT::cleanSession() const noexcept
{
  return m_cleanSession;
}

/**
 * @brief Returns the broker TCP port.
 */
quint16 IO::Drivers::MQTT::port() const noexcept
{
  return m_port;
}

/**
 * @brief Returns the keep-alive interval in seconds.
 */
quint16 IO::Drivers::MQTT::keepAlive() const noexcept
{
  return m_keepAlive;
}

/**
 * @brief Returns the index of the selected MQTT protocol version.
 */
quint8 IO::Drivers::MQTT::mqttVersion() const noexcept
{
  quint8 index = 0;
  for (auto i = m_mqttVersions.begin(); i != m_mqttVersions.end(); ++i) {
    if (i.value() == m_protocolVersion)
      break;

    ++index;
  }

  return index;
}

/**
 * @brief Returns the index of the selected SSL protocol.
 */
quint8 IO::Drivers::MQTT::sslProtocol() const noexcept
{
  quint8 index = 0;
  for (auto i = m_sslProtocols.begin(); i != m_sslProtocols.end(); ++i) {
    if (i.value() == m_sslConfiguration.protocol())
      break;

    ++index;
  }

  return index;
}

/**
 * @brief Returns the index of the selected peer verification mode.
 */
quint8 IO::Drivers::MQTT::peerVerifyMode() const noexcept
{
  quint8 index = 0;
  for (auto i = m_peerVerifyModes.begin(); i != m_peerVerifyModes.end(); ++i) {
    if (i.value() == m_sslConfiguration.peerVerifyMode())
      break;

    ++index;
  }

  return index;
}

/**
 * @brief Returns the maximum SSL peer-verification chain depth.
 */
int IO::Drivers::MQTT::peerVerifyDepth() const noexcept
{
  return m_sslConfiguration.peerVerifyDepth();
}

/**
 * @brief Returns the MQTT client identifier.
 */
QString IO::Drivers::MQTT::clientId() const
{
  return m_clientId;
}

/**
 * @brief Returns the configured broker hostname.
 */
QString IO::Drivers::MQTT::hostname() const
{
  return m_hostname;
}

/**
 * @brief Returns the broker authentication username.
 */
QString IO::Drivers::MQTT::username() const
{
  return m_username;
}

/**
 * @brief Returns the broker authentication password.
 */
QString IO::Drivers::MQTT::password() const
{
  return m_password;
}

/**
 * @brief Returns the active topic filter.
 */
QString IO::Drivers::MQTT::topicFilter() const
{
  return m_topicFilter;
}

/**
 * @brief Returns the client certificate PEM path used for mutual TLS (empty = off).
 */
QString IO::Drivers::MQTT::clientCertificatePath() const
{
  return m_clientCertificatePath;
}

/**
 * @brief Returns the private key PEM path (empty = look in the certificate file).
 */
QString IO::Drivers::MQTT::privateKeyPath() const
{
  return m_privateKeyPath;
}

/**
 * @brief Returns the private-key passphrase (kept in the encrypted vault, never in QSettings).
 */
QString IO::Drivers::MQTT::keyPassphrase() const
{
  return m_keyPassphrase;
}

/**
 * @brief Returns whether ALPN is requested during the TLS handshake (MQTT over port 443).
 */
bool IO::Drivers::MQTT::alpnEnabled() const noexcept
{
  return m_alpnEnabled;
}

/**
 * @brief Returns the ALPN protocol name announced when ALPN is enabled.
 */
QString IO::Drivers::MQTT::alpnProtocol() const
{
  return m_alpnProtocol;
}

/**
 * @brief Returns the available MQTT protocol versions (display names).
 */
const QStringList& IO::Drivers::MQTT::mqttVersions() const
{
  static QStringList list;
  if (list.isEmpty())
    for (auto i = m_mqttVersions.begin(); i != m_mqttVersions.end(); ++i)
      list.append(i.key());

  return list;
}

/**
 * @brief Returns the available SSL/TLS protocol names.
 */
const QStringList& IO::Drivers::MQTT::sslProtocols() const
{
  static QStringList list;
  if (list.isEmpty())
    for (auto i = m_sslProtocols.begin(); i != m_sslProtocols.end(); ++i)
      list.append(i.key());

  return list;
}

/**
 * @brief Returns the available SSL peer-verification modes.
 */
const QStringList& IO::Drivers::MQTT::peerVerifyModes() const
{
  static QStringList list;
  if (list.isEmpty())
    for (auto i = m_peerVerifyModes.begin(); i != m_peerVerifyModes.end(); ++i)
      list.append(i.key());

  return list;
}

/**
 * @brief Returns the available CA certificate sources (system DB or folder).
 */
const QStringList& IO::Drivers::MQTT::caCertificates() const
{
  static QStringList list;
  if (list.isEmpty()) {
    list.append(tr("Use System Database"));
    list.append(tr("Load From Folder…"));
  }

  return list;
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Generates a random 16-character lowercase client ID.
 */
void IO::Drivers::MQTT::regenerateClientId()
{
  QString id;
  constexpr int length  = 16;
  const QString charset = QStringLiteral("abcdefghijklmnopqrstuvwxyz0123456789");
  for (int i = 0; i < length; ++i) {
    const int index = QRandomGenerator::global()->bounded(charset.length());
    id.append(charset.at(index));
  }

  setClientId(id);
}

/**
 * @brief Opens a folder picker to load additional CA certificates.
 */
void IO::Drivers::MQTT::addCaCertificates()
{
  auto* dialog =
    new QFileDialog(qApp->activeWindow(),
                    tr("Select PEM Certificates Directory"),
                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

  dialog->setFileMode(QFileDialog::Directory);
  dialog->setOption(QFileDialog::ShowDirsOnly, true);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this, [this, path]() { m_sslConfiguration.addCaCertificates(path); }, Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Opens a PEM file picker and routes the selection into the given path setter. The work
 *        runs through a queued invoke: on macOS fileSelected fires inside QFileDialog::done()
 *        and re-entering Qt synchronously can delete the dialog under the native panel.
 */
void IO::Drivers::MQTT::selectPemFile(const QString& title, void (MQTT::*setter)(const QString&))
{
  SS_ASSERT(setter != nullptr, return);
  SS_ASSERT(!title.isEmpty(), return);

  auto* dialog =
    new QFileDialog(qApp->activeWindow(),
                    title,
                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                    tr("PEM files (*.pem *.crt *.cer *.key);;All files (*)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this, setter](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this,
      [this, setter, path]() {
        (this->*setter)(path);
        reloadTlsIdentity(true);
      },
      Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Opens a file picker for the mutual-TLS client certificate.
 */
void IO::Drivers::MQTT::selectClientCertificate()
{
  selectPemFile(tr("Select Client Certificate"), &MQTT::setClientCertificatePath);
}

/**
 * @brief Opens a file picker for the mutual-TLS private key.
 */
void IO::Drivers::MQTT::selectPrivateKey()
{
  selectPemFile(tr("Select Private Key"), &MQTT::setPrivateKeyPath);
}

/**
 * @brief Sets the broker TCP port.
 */
void IO::Drivers::MQTT::setPort(const quint16 port)
{
  if (m_port == port)
    return;

  m_port = port;
  m_settings.setValue(settingsKey("port"), port);
  scheduleReconnectIfActive();
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the keep-alive interval in seconds.
 */
void IO::Drivers::MQTT::setKeepAlive(const quint16 keepAlive)
{
  if (m_keepAlive == keepAlive)
    return;

  m_keepAlive = keepAlive;
  m_settings.setValue(settingsKey("keepAlive"), keepAlive);
  scheduleReconnectIfActive();
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Enables or disables automatic keep-alive ping handling.
 */
void IO::Drivers::MQTT::setAutoKeepAlive(const bool autoKeepAlive)
{
  if (m_autoKeepAlive == autoKeepAlive)
    return;

  m_autoKeepAlive = autoKeepAlive;
  m_settings.setValue(settingsKey("autoKeepAlive"), autoKeepAlive);
  scheduleReconnectIfActive();
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Enables or disables the clean-session flag.
 */
void IO::Drivers::MQTT::setCleanSession(const bool cleanSession)
{
  if (m_cleanSession == cleanSession)
    return;

  m_cleanSession = cleanSession;
  m_settings.setValue(settingsKey("cleanSession"), cleanSession);
  scheduleReconnectIfActive();
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the MQTT protocol version by index.
 */
void IO::Drivers::MQTT::setMqttVersion(const quint8 version)
{
  quint8 index = 0;
  for (auto i = m_mqttVersions.begin(); i != m_mqttVersions.end(); ++i) {
    if (index == version) {
      if (i.value() == m_protocolVersion)
        return;

      m_protocolVersion = i.value();
      m_settings.setValue(settingsKey("mqttVersion"), version);
      scheduleReconnectIfActive();
      Q_EMIT mqttConfigurationChanged();
      return;
    }

    ++index;
  }
}

/**
 * @brief Enables or disables SSL/TLS for the broker connection.
 */
void IO::Drivers::MQTT::setSslEnabled(const bool enabled)
{
  if (m_sslEnabled == enabled)
    return;

  m_sslEnabled = enabled;
  m_settings.setValue(settingsKey("sslEnabled"), enabled);
  scheduleReconnectIfActive();
  Q_EMIT sslConfigurationChanged();
}

/**
 * @brief Sets the SSL protocol by index.
 */
void IO::Drivers::MQTT::setSslProtocol(const quint8 protocol)
{
  quint8 index = 0;
  for (auto i = m_sslProtocols.begin(); i != m_sslProtocols.end(); ++i) {
    if (index == protocol) {
      if (i.value() == m_sslConfiguration.protocol())
        return;

      m_sslConfiguration.setProtocol(i.value());
      m_settings.setValue(settingsKey("sslProtocol"), protocol);
      scheduleReconnectIfActive();
      Q_EMIT sslConfigurationChanged();
      return;
    }

    ++index;
  }
}

/**
 * @brief Sets the peer-verification mode by index.
 */
void IO::Drivers::MQTT::setPeerVerifyMode(const quint8 verifyMode)
{
  quint8 index = 0;
  for (auto i = m_peerVerifyModes.begin(); i != m_peerVerifyModes.end(); ++i) {
    if (index == verifyMode) {
      if (i.value() == m_sslConfiguration.peerVerifyMode())
        return;

      if (i.value() == QSslSocket::VerifyNone) [[unlikely]]
        qWarning()
          << "[MQTT input] TLS peer verification disabled -- connection vulnerable to MITM";

      m_sslConfiguration.setPeerVerifyMode(i.value());
      m_settings.setValue(settingsKey("peerVerifyMode"), verifyMode);
      scheduleReconnectIfActive();
      Q_EMIT sslConfigurationChanged();
      return;
    }

    ++index;
  }
}

/**
 * @brief Sets the maximum SSL peer-verification chain depth.
 */
void IO::Drivers::MQTT::setPeerVerifyDepth(const int depth)
{
  if (m_sslConfiguration.peerVerifyDepth() == depth)
    return;

  m_sslConfiguration.setPeerVerifyDepth(depth);
  m_settings.setValue(settingsKey("peerVerifyDepth"), depth);
  scheduleReconnectIfActive();
  Q_EMIT sslConfigurationChanged();
}

/**
 * @brief Sets the MQTT client identifier.
 */
void IO::Drivers::MQTT::setClientId(const QString& id)
{
  if (m_clientId == id)
    return;

  m_clientId = id;
  m_settings.setValue(settingsKey("clientId"), id);
  scheduleReconnectIfActive();
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the broker hostname.
 */
void IO::Drivers::MQTT::setHostname(const QString& hostname)
{
  if (m_hostname == hostname)
    return;

  m_hostname = hostname;
  m_settings.setValue(settingsKey("hostname"), hostname);
  scheduleReconnectIfActive();
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the broker authentication username.
 */
void IO::Drivers::MQTT::setUsername(const QString& username)
{
  if (m_username == username)
    return;

  m_username = username;
  m_vault.setCredentials(m_hostname, m_port, m_username, m_password);
  scheduleReconnectIfActive();
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the broker authentication password.
 */
void IO::Drivers::MQTT::setPassword(const QString& password)
{
  if (m_password == password)
    return;

  m_password = password;
  m_vault.setCredentials(m_hostname, m_port, m_username, m_password);
  scheduleReconnectIfActive();
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the client certificate PEM path and reloads the parsed TLS identity.
 */
void IO::Drivers::MQTT::setClientCertificatePath(const QString& path)
{
  if (m_clientCertificatePath == path)
    return;

  m_clientCertificatePath = path;
  m_settings.setValue(settingsKey("clientCertPath"), path);
  reloadTlsIdentity(false);
  scheduleReconnectIfActive();
  Q_EMIT sslConfigurationChanged();
}

/**
 * @brief Sets the private key PEM path and reloads the parsed TLS identity.
 */
void IO::Drivers::MQTT::setPrivateKeyPath(const QString& path)
{
  if (m_privateKeyPath == path)
    return;

  m_privateKeyPath = path;
  m_settings.setValue(settingsKey("privateKeyPath"), path);
  reloadTlsIdentity(false);
  scheduleReconnectIfActive();
  Q_EMIT sslConfigurationChanged();
}

/**
 * @brief Sets the private-key passphrase, persists it to the vault and re-parses the key.
 */
void IO::Drivers::MQTT::setKeyPassphrase(const QString& passphrase)
{
  if (m_keyPassphrase == passphrase)
    return;

  m_keyPassphrase = passphrase;
  m_vault.setKeyPassphrase(m_hostname, m_port, m_keyPassphrase);
  reloadTlsIdentity(false);
  scheduleReconnectIfActive();
  Q_EMIT sslConfigurationChanged();
}

/**
 * @brief Enables or disables ALPN announcement during the TLS handshake. Re-applies the cached
 *        identity instead of re-parsing the PEM files: an ALPN edit cannot change them, and a
 *        reload here would drop a valid in-memory identity if the files moved since load.
 */
void IO::Drivers::MQTT::setAlpnEnabled(const bool enabled)
{
  if (m_alpnEnabled == enabled)
    return;

  m_alpnEnabled = enabled;
  m_settings.setValue(settingsKey("alpnEnabled"), enabled);
  ::MQTT::applyTlsIdentity(
    m_sslConfiguration, m_tlsIdentity, m_alpnEnabled ? m_alpnProtocol.toUtf8() : QByteArray());
  scheduleReconnectIfActive();
  Q_EMIT sslConfigurationChanged();
}

/**
 * @brief Sets the ALPN protocol name (AWS IoT uses "x-amzn-mqtt-ca" on port 443). Same cached
 *        re-apply as setAlpnEnabled.
 */
void IO::Drivers::MQTT::setAlpnProtocol(const QString& protocol)
{
  if (m_alpnProtocol == protocol)
    return;

  m_alpnProtocol = protocol;
  m_settings.setValue(settingsKey("alpnProtocol"), protocol);
  ::MQTT::applyTlsIdentity(
    m_sslConfiguration, m_tlsIdentity, m_alpnEnabled ? m_alpnProtocol.toUtf8() : QByteArray());
  scheduleReconnectIfActive();
  Q_EMIT sslConfigurationChanged();
}

/**
 * @brief Sets the MQTT topic filter used for subscription.
 */
void IO::Drivers::MQTT::setTopicFilter(const QString& topic)
{
  if (m_topicFilter == topic)
    return;

  m_topicFilter = topic;
  m_settings.setValue(settingsKey("topicFilter"), topic);
  scheduleReconnectIfActive();
  Q_EMIT mqttConfigurationChanged();
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the MQTT input configuration as a flat list of editable properties.
 */
QList<IO::DriverProperty> IO::Drivers::MQTT::driverProperties() const
{
  QList<IO::DriverProperty> props;

  IO::DriverProperty host;
  host.key   = QStringLiteral("hostname");
  host.label = tr("Hostname");
  host.type  = IO::DriverProperty::Text;
  host.value = m_hostname;
  props.append(host);

  IO::DriverProperty p;
  p.key   = QStringLiteral("port");
  p.label = tr("Port");
  p.type  = IO::DriverProperty::IntField;
  p.value = m_port;
  p.min   = 1;
  p.max   = 65535;
  props.append(p);

  IO::DriverProperty topic;
  topic.key   = QStringLiteral("topicFilter");
  topic.label = tr("Topic Filter");
  topic.type  = IO::DriverProperty::Text;
  topic.value = m_topicFilter;
  props.append(topic);

  IO::DriverProperty cid;
  cid.key   = QStringLiteral("clientId");
  cid.label = tr("Client ID");
  cid.type  = IO::DriverProperty::Text;
  cid.value = m_clientId;
  props.append(cid);

  IO::DriverProperty user;
  user.key   = QStringLiteral("username");
  user.label = tr("Username");
  user.type  = IO::DriverProperty::Text;
  user.value = m_username;
  props.append(user);

  IO::DriverProperty pass;
  pass.key   = QStringLiteral("password");
  pass.label = tr("Password");
  pass.type  = IO::DriverProperty::Password;
  pass.value = m_password;
  props.append(pass);

  IO::DriverProperty ver;
  ver.key     = QStringLiteral("mqttVersion");
  ver.label   = tr("MQTT Version");
  ver.type    = IO::DriverProperty::ComboBox;
  ver.value   = mqttVersion();
  ver.options = mqttVersions();
  props.append(ver);

  IO::DriverProperty clean;
  clean.key   = QStringLiteral("cleanSession");
  clean.label = tr("Clean Session");
  clean.type  = IO::DriverProperty::CheckBox;
  clean.value = m_cleanSession;
  props.append(clean);

  IO::DriverProperty ka;
  ka.key   = QStringLiteral("keepAlive");
  ka.label = tr("Keep Alive (s)");
  ka.type  = IO::DriverProperty::IntField;
  ka.value = m_keepAlive;
  ka.min   = 0;
  ka.max   = 65535;
  props.append(ka);

  IO::DriverProperty autoKa;
  autoKa.key   = QStringLiteral("autoKeepAlive");
  autoKa.label = tr("Auto Keep Alive");
  autoKa.type  = IO::DriverProperty::CheckBox;
  autoKa.value = m_autoKeepAlive;
  props.append(autoKa);

  appendMqttSslProperties(props);

  return props;
}

/**
 * @brief Appends SSL/TLS toggle and (when enabled) protocol, peer verify mode, depth, and the
 *        mutual-TLS rows. The key passphrase is deliberately absent: property values are
 *        snapshotted into project files as plaintext, so it lives only in the encrypted vault.
 */
void IO::Drivers::MQTT::appendMqttSslProperties(QList<IO::DriverProperty>& props) const
{
  IO::DriverProperty ssl;
  ssl.key   = QStringLiteral("sslEnabled");
  ssl.label = tr("SSL/TLS Enabled");
  ssl.type  = IO::DriverProperty::CheckBox;
  ssl.value = m_sslEnabled;
  props.append(ssl);

  if (!m_sslEnabled)
    return;

  IO::DriverProperty proto;
  proto.key     = QStringLiteral("sslProtocol");
  proto.label   = tr("SSL Protocol");
  proto.type    = IO::DriverProperty::ComboBox;
  proto.value   = sslProtocol();
  proto.options = sslProtocols();
  props.append(proto);

  IO::DriverProperty mode;
  mode.key     = QStringLiteral("peerVerifyMode");
  mode.label   = tr("Peer Verify Mode");
  mode.type    = IO::DriverProperty::ComboBox;
  mode.value   = peerVerifyMode();
  mode.options = peerVerifyModes();
  props.append(mode);

  IO::DriverProperty depth;
  depth.key   = QStringLiteral("peerVerifyDepth");
  depth.label = tr("Peer Verify Depth");
  depth.type  = IO::DriverProperty::IntField;
  depth.value = m_sslConfiguration.peerVerifyDepth();
  depth.min   = 0;
  depth.max   = 100;
  props.append(depth);

  IO::DriverProperty cert;
  cert.key   = QStringLiteral("clientCertificatePath");
  cert.label = tr("Client Certificate (PEM)");
  cert.type  = IO::DriverProperty::Text;
  cert.value = m_clientCertificatePath;
  props.append(cert);

  IO::DriverProperty key;
  key.key   = QStringLiteral("privateKeyPath");
  key.label = tr("Private Key (PEM)");
  key.type  = IO::DriverProperty::Text;
  key.value = m_privateKeyPath;
  props.append(key);

  IO::DriverProperty alpn;
  alpn.key   = QStringLiteral("alpnEnabled");
  alpn.label = tr("ALPN (MQTT over port 443)");
  alpn.type  = IO::DriverProperty::CheckBox;
  alpn.value = m_alpnEnabled;
  props.append(alpn);

  if (m_alpnEnabled) {
    IO::DriverProperty proto443;
    proto443.key   = QStringLiteral("alpnProtocol");
    proto443.label = tr("ALPN Protocol");
    proto443.type  = IO::DriverProperty::Text;
    proto443.value = m_alpnProtocol;
    props.append(proto443);
  }
}

/**
 * @brief Applies a single MQTT input configuration change by key.
 */
void IO::Drivers::MQTT::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("hostname")) {
    setHostname(value.toString());
    return;
  }

  if (key == QLatin1String("port")) {
    setPort(static_cast<quint16>(value.toInt()));
    return;
  }

  if (key == QLatin1String("topicFilter")) {
    setTopicFilter(value.toString());
    return;
  }

  if (key == QLatin1String("clientId")) {
    setClientId(value.toString());
    return;
  }

  if (key == QLatin1String("username")) {
    setUsername(value.toString());
    return;
  }

  if (key == QLatin1String("password")) {
    setPassword(value.toString());
    return;
  }

  if (key == QLatin1String("mqttVersion")) {
    setMqttVersion(static_cast<quint8>(value.toInt()));
    return;
  }

  if (key == QLatin1String("cleanSession")) {
    setCleanSession(value.toBool());
    return;
  }

  if (key == QLatin1String("keepAlive")) {
    setKeepAlive(static_cast<quint16>(value.toInt()));
    return;
  }

  if (key == QLatin1String("autoKeepAlive")) {
    setAutoKeepAlive(value.toBool());
    return;
  }

  if (key == QLatin1String("sslEnabled")) {
    setSslEnabled(value.toBool());
    return;
  }

  if (key == QLatin1String("sslProtocol")) {
    setSslProtocol(static_cast<quint8>(value.toInt()));
    return;
  }

  if (key == QLatin1String("peerVerifyMode")) {
    setPeerVerifyMode(static_cast<quint8>(value.toInt()));
    return;
  }

  if (key == QLatin1String("peerVerifyDepth")) {
    setPeerVerifyDepth(value.toInt());
    return;
  }

  if (key == QLatin1String("clientCertificatePath")) {
    setClientCertificatePath(value.toString());
    return;
  }

  if (key == QLatin1String("privateKeyPath")) {
    setPrivateKeyPath(value.toString());
    return;
  }

  if (key == QLatin1String("keyPassphrase")) {
    setKeyPassphrase(value.toString());
    return;
  }

  if (key == QLatin1String("alpnEnabled")) {
    setAlpnEnabled(value.toBool());
    return;
  }

  if (key == QLatin1String("alpnProtocol"))
    setAlpnProtocol(value.toString());
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles broker state transitions. A session that goes down while the user still wants it
 *        open, and outside an attempt of our own, is the drop a supervised flow recovers from.
 */
void IO::Drivers::MQTT::onStateChanged(QMqttClient::ClientState state)
{
  qCInfo(lcMqttSub) << "state changed:" << state;
  Q_EMIT connectedChanged();

  if (state == QMqttClient::Connected) {
    m_failureNotified    = false;
    m_sessionEstablished = true;
    return;
  }

  if (state == QMqttClient::Disconnected && m_userWantsOpen && !m_connecting)
    Q_EMIT linkDropped();
}

/**
 * @brief Surfaces broker errors to the user, subject to claimFailureReport(): a retried connect
 *        reports the same failure on every attempt, and a session that had been up hands its
 *        failure to the shared give-up report instead of boxing here as well.
 */
void IO::Drivers::MQTT::onErrorChanged(QMqttClient::ClientError error)
{
  if (error == QMqttClient::NoError)
    return;

  qCWarning(lcMqttSub) << "client error" << error;
  if (!claimFailureReport())
    return;

  QString title;
  QString message;
  switch (error) {
    case QMqttClient::InvalidProtocolVersion:
      title   = tr("Invalid MQTT Protocol Version");
      message = tr("The broker rejected the configured MQTT protocol version.");
      break;
    case QMqttClient::IdRejected:
      title   = tr("Client ID Rejected");
      message = tr("The broker rejected the client ID. Try a different identifier.");
      break;
    case QMqttClient::ServerUnavailable:
      title   = tr("MQTT Server Unavailable");
      message = tr("The broker is currently unavailable. Retry later.");
      break;
    case QMqttClient::BadUsernameOrPassword:
      title   = tr("Authentication Error");
      message = tr("The credentials provided were rejected by the broker.");
      break;
    case QMqttClient::NotAuthorized:
      title   = tr("Authorization Error");
      message = tr("Account lacks permission for this operation.");
      break;
    case QMqttClient::TransportInvalid:
      title   = tr("Network or Transport Error");
      message = tr("Network/transport layer issue while connecting to the broker.");
      if (!m_tlsIdentity.certificate.isNull())
        message += QStringLiteral(" ")
                 + tr("A client certificate is configured: verify that it matches the private "
                      "key and is activated on the broker.");

      break;
    case QMqttClient::ProtocolViolation:
      title   = tr("MQTT Protocol Violation");
      message = tr("The broker reported a protocol violation and closed the connection.");
      break;
    case QMqttClient::Mqtt5SpecificError:
      title   = tr("MQTT 5 Error");
      message = tr("An MQTT 5 protocol-level error occurred.");
      break;
    default:
      title   = tr("MQTT Error");
      message = tr("An unexpected MQTT error occurred.");
      break;
  }

  Misc::Utilities::showMessageBox(title, message, QMessageBox::Critical);
}

/**
 * @brief Forwards a received MQTT message into the frame-reader pipeline.
 */
void IO::Drivers::MQTT::onMessageReceived(const QByteArray& message, const QMqttTopicName& topic)
{
  SS_ASSERT(topic.isValid(), return);

  const auto& token = Licensing::CommercialToken::current();
  if (!token.isValid() || !SS_LICENSE_GUARD()) {
    qCWarning(lcMqttSub) << "messageReceived dropped: no commercial license";
    return;
  }

  if (message.isEmpty()) {
    qCInfo(lcMqttSub) << "messageReceived: empty payload on" << topic.name() << "-- dropped";
    return;
  }

  QMqttTopicFilter filter(m_topicFilter);
  if (!filter.match(topic)) {
    qCInfo(lcMqttSub) << "messageReceived: topic" << topic.name() << "did not match filter"
                      << m_topicFilter << "-- dropped";
    return;
  }

  qCDebug(lcMqttSub) << "messageReceived on" << topic.name() << "size=" << message.size()
                     << "preview=" << message.left(80);

  publishReceivedData(message);
}

//--------------------------------------------------------------------------------------------------
// Persistence helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Restores broker configuration from QSettings under the driver namespace.
 */
void IO::Drivers::MQTT::loadPersistedSettings()
{
  const auto host = m_settings.value(settingsKey("hostname"), m_hostname).toString();
  const auto cid  = m_settings.value(settingsKey("clientId"), QString()).toString();
  const auto top  = m_settings.value(settingsKey("topicFilter"), QString()).toString();

  const auto p    = m_settings.value(settingsKey("port"), m_port).toUInt();
  const auto ka   = m_settings.value(settingsKey("keepAlive"), m_keepAlive).toUInt();
  const auto ver  = m_settings.value(settingsKey("mqttVersion"), mqttVersion()).toUInt();
  const auto pvd  = m_settings.value(settingsKey("peerVerifyDepth"), 10).toInt();
  const auto sslP = m_settings.value(settingsKey("sslProtocol"), sslProtocol()).toUInt();
  const auto pvm  = m_settings.value(settingsKey("peerVerifyMode"), peerVerifyMode()).toUInt();

  const auto autoKa = m_settings.value(settingsKey("autoKeepAlive"), m_autoKeepAlive).toBool();
  const auto clean  = m_settings.value(settingsKey("cleanSession"), m_cleanSession).toBool();
  const auto ssl    = m_settings.value(settingsKey("sslEnabled"), false).toBool();

  const auto port16        = static_cast<quint16>(p);
  auto creds               = m_vault.credentials(host, port16);
  const bool hasLegacyUser = m_settings.contains(settingsKey("username"));
  const bool hasLegacyPass = m_settings.contains(settingsKey("password"));
  if ((hasLegacyUser || hasLegacyPass) && creds.username.isEmpty() && creds.password.isEmpty()) {
    creds.username = m_settings.value(settingsKey("username"), QString()).toString();
    creds.password = m_settings.value(settingsKey("password"), QString()).toString();
    m_vault.setCredentials(host, port16, creds.username, creds.password);
  }
  if (hasLegacyUser)
    m_settings.remove(settingsKey("username"));

  if (hasLegacyPass)
    m_settings.remove(settingsKey("password"));

  setHostname(host);
  setPort(port16);
  setClientId(cid);
  setUsername(creds.username);
  setPassword(creds.password);
  setTopicFilter(top);
  setKeepAlive(static_cast<quint16>(ka));
  setAutoKeepAlive(autoKa);
  setCleanSession(clean);
  setMqttVersion(static_cast<quint8>(ver));
  setSslEnabled(ssl);
  setSslProtocol(static_cast<quint8>(sslP));
  setPeerVerifyMode(static_cast<quint8>(pvm));
  setPeerVerifyDepth(pvd);

  m_keyPassphrase = m_vault.keyPassphrase(host, port16);
  m_alpnEnabled   = m_settings.value(settingsKey("alpnEnabled"), false).toBool();
  m_alpnProtocol =
    m_settings.value(settingsKey("alpnProtocol"), QStringLiteral("x-amzn-mqtt-ca")).toString();
  m_clientCertificatePath = m_settings.value(settingsKey("clientCertPath"), QString()).toString();
  m_privateKeyPath        = m_settings.value(settingsKey("privateKeyPath"), QString()).toString();
  reloadTlsIdentity(false);
  Q_EMIT sslConfigurationChanged();
}

/**
 * @brief Builds a fully-qualified QSettings key under the MqttInputDriver namespace.
 */
QString IO::Drivers::MQTT::settingsKey(const char* leaf) const
{
  return QStringLiteral("MqttInputDriver/") + QLatin1String(leaf);
}

/**
 * @brief Pushes the mirror snapshot into m_client. Caller must guarantee state == Disconnected.
 */
void IO::Drivers::MQTT::applyPendingToClient()
{
  SS_ASSERT(m_client.state() == QMqttClient::Disconnected, return);

  m_client.setHostname(m_hostname);
  m_client.setPort(m_port);
  m_client.setClientId(m_clientId);
  m_client.setUsername(m_username);
  m_client.setPassword(m_password);
  m_client.setKeepAlive(m_keepAlive);
  m_client.setAutoKeepAlive(m_autoKeepAlive);
  m_client.setCleanSession(m_cleanSession);
  m_client.setProtocolVersion(m_protocolVersion);
}

/**
 * @brief Re-parses the client certificate + key pair and applies the result (plus ALPN) to the
 *        driver's SSL configuration. A failed parse clears the identity so a stale pair is never
 *        sent; interactive callers get a message box, restore paths only log.
 */
void IO::Drivers::MQTT::reloadTlsIdentity(const bool interactive)
{
  const auto result = ::MQTT::loadTlsIdentity(
    m_clientCertificatePath, m_privateKeyPath, m_keyPassphrase, m_tlsIdentity);

  const auto alpn = m_alpnEnabled ? m_alpnProtocol.toUtf8() : QByteArray();
  ::MQTT::applyTlsIdentity(m_sslConfiguration, m_tlsIdentity, alpn);

  if (result.ok())
    return;

  qCWarning(lcMqttSub) << "TLS identity rejected:" << ::MQTT::tlsIdentityErrorString(result);
  if (interactive)
    Misc::Utilities::showMessageBox(tr("MQTT Client Certificate Error"),
                                    ::MQTT::tlsIdentityErrorString(result),
                                    QMessageBox::Warning);
}

/**
 * @brief If the live connection is active, drop it now and reopen through the flow. Running it on
 *        the driver's runner is what supersedes an earlier reconnect instead of stacking one-shot
 *        connections, and the drop itself is not reported as a link failure.
 */
void IO::Drivers::MQTT::scheduleReconnectIfActive()
{
  if (m_client.state() == QMqttClient::Disconnected)
    return;

  qCInfo(lcMqttSub) << "broker setting changed while connected -- scheduling reconnect";

  m_connecting = true;
  m_runner.cancel();
  m_client.disconnectFromHost();
  m_runner.run(buildReconnectFlow());
}

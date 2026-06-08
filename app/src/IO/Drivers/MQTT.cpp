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

Q_LOGGING_CATEGORY(lcMqttSub, "serialstudio.mqtt.subscriber", QtCriticalMsg)

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
  , m_reconnectPending(false)
  , m_port(1883)
  , m_keepAlive(60)
  , m_protocolVersion(QMqttClient::MQTT_5_0)
  , m_hostname(QStringLiteral("127.0.0.1"))
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
 * @brief Disconnects from the MQTT broker.
 */
void IO::Drivers::MQTT::close()
{
  m_userWantsOpen = false;

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
 * @brief Opens the broker connection. Subscription is issued once Connected.
 */
bool IO::Drivers::MQTT::open(const QIODevice::OpenMode mode)
{
  Q_UNUSED(mode);

  const auto& token = Licensing::CommercialToken::current();
  if (!token.isValid() || !SS_LICENSE_GUARD()
      || token.featureTier() < Licensing::FeatureTier::Trial) {
    Misc::Utilities::showMessageBox(
      tr("MQTT Feature Requires a Commercial License"),
      tr("Subscribing to an MQTT broker is only available with a valid Serial Studio license "
         "or an active trial."),
      QMessageBox::Warning);
    return false;
  }

  if (m_hostname.isEmpty() || m_port == 0) {
    qCWarning(lcMqttSub) << "open() rejected: missing hostname or port" << m_hostname << m_port;
    return false;
  }

  if (m_topicFilter.isEmpty()) {
    qCWarning(lcMqttSub) << "open() rejected: empty topic filter";
    return false;
  }

  if (m_client.state() != QMqttClient::Disconnected) {
    qCInfo(lcMqttSub) << "open() no-op; state already" << m_client.state();
    return true;
  }

  if (m_clientId.isEmpty())
    regenerateClientId();

  applyPendingToClient();
  m_userWantsOpen = true;

  qCInfo(lcMqttSub).nospace() << "Connecting to " << (m_sslEnabled ? "mqtts://" : "mqtt://")
                              << m_hostname << ":" << m_port << " clientId=" << m_clientId
                              << " topic=" << m_topicFilter;

  if (m_sslEnabled)
    m_client.connectToHostEncrypted(m_sslConfiguration);
  else
    m_client.connectToHost();

  return true;
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
 * @brief Appends SSL/TLS toggle and (when enabled) protocol, peer verify mode, and depth.
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

  if (key == QLatin1String("peerVerifyDepth"))
    setPeerVerifyDepth(value.toInt());
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles broker state transitions; subscribes once Connected.
 */
void IO::Drivers::MQTT::onStateChanged(QMqttClient::ClientState state)
{
  qCInfo(lcMqttSub) << "state changed:" << state;
  Q_EMIT connectedChanged();

  if (state == QMqttClient::Connected && !m_topicFilter.isEmpty()) {
    QMqttTopicFilter filter;
    filter.setFilter(m_topicFilter);

    auto* sub = m_client.subscribe(filter, 0);
    if (!sub || sub->state() == QMqttSubscription::Error) {
      qCCritical(lcMqttSub) << "subscribe failed for filter" << m_topicFilter;
      Misc::Utilities::showMessageBox(tr("MQTT Subscription Error"),
                                      tr("Failed to subscribe to topic \"%1\".").arg(m_topicFilter),
                                      QMessageBox::Critical);
      return;
    }

    qCInfo(lcMqttSub) << "subscribed to" << m_topicFilter << "initial state:" << sub->state();
    connect(
      sub, &QMqttSubscription::stateChanged, this, [this](QMqttSubscription::SubscriptionState s) {
        qCInfo(lcMqttSub) << "subscription state for" << m_topicFilter << "->" << s;
      });
  }
}

/**
 * @brief Surfaces broker errors to the user.
 */
void IO::Drivers::MQTT::onErrorChanged(QMqttClient::ClientError error)
{
  if (error != QMqttClient::NoError)
    qCWarning(lcMqttSub) << "client error" << error;

  QString title;
  QString message;
  switch (error) {
    case QMqttClient::NoError:
      return;
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
  Q_ASSERT(topic.isValid());

  const auto& token = Licensing::CommercialToken::current();
  if (!token.isValid() || !SS_LICENSE_GUARD()
      || token.featureTier() < Licensing::FeatureTier::Trial) {
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
  Q_ASSERT(m_client.state() == QMqttClient::Disconnected);

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
 * @brief If the live connection is active, disconnect now and reopen once Disconnected.
 */
void IO::Drivers::MQTT::scheduleReconnectIfActive()
{
  if (m_client.state() == QMqttClient::Disconnected)
    return;

  if (m_reconnectPending) {
    if (m_client.state() != QMqttClient::Disconnected)
      m_client.disconnectFromHost();

    return;
  }

  qCInfo(lcMqttSub) << "broker setting changed while connected -- scheduling reconnect";
  m_reconnectPending = true;

  auto* conn = new QMetaObject::Connection;
  *conn =
    connect(&m_client, &QMqttClient::stateChanged, this, [this, conn](QMqttClient::ClientState s) {
      if (s != QMqttClient::Disconnected)
        return;

      disconnect(*conn);
      delete conn;
      m_reconnectPending = false;

      if (!m_userWantsOpen)
        return;

      (void)open(QIODevice::ReadOnly);
    });

  m_client.disconnectFromHost();
}

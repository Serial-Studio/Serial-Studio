/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef USE_QT_COMMERCIAL

// clang-format off
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
// clang-format on

// clang-format off
#include "IO/Manager.h"
#include "MQTT/Client.h"
#include "Misc/Utilities.h"
// clang-format on

//------------------------------------------------------------------------------
// Constructor & singleton access functions
//------------------------------------------------------------------------------

MQTT::Client::Client()
  : m_publisher(false)
  , m_sslEnabled(false)
{
  // Initialize MQTT versions model
  m_mqttVersions.insert(tr("MQTT 3.1"), QMqttClient::MQTT_3_1);
  m_mqttVersions.insert(tr("MQTT 3.1.1"), QMqttClient::MQTT_3_1_1);
  m_mqttVersions.insert(tr("MQTT 5.0"), QMqttClient::MQTT_5_0);

  // Initialize SSL protocols model
  m_sslProtocols.insert(tr("TLS 1.2"), QSsl::TlsV1_2);
  m_sslProtocols.insert(tr("TLS 1.3"), QSsl::TlsV1_3);
  m_sslProtocols.insert(tr("TLS 1.3 or Later"), QSsl::TlsV1_3OrLater);
  m_sslProtocols.insert(tr("DTLS 1.2 or Later"), QSsl::DtlsV1_2OrLater);
  m_sslProtocols.insert(tr("Any Protocol"), QSsl::AnyProtocol);
  m_sslProtocols.insert(tr("Secure Protocols Only"), QSsl::SecureProtocols);

  // Initialize SSL peer verify modes model
  m_peerVerifyModes.insert(tr("None"), QSslSocket::VerifyNone);
  m_peerVerifyModes.insert(tr("Query Peer"), QSslSocket::QueryPeer);
  m_peerVerifyModes.insert(tr("Verify Peer"), QSslSocket::VerifyPeer);
  m_peerVerifyModes.insert(tr("Auto Verify Peer"), QSslSocket::AutoVerifyPeer);

  // Set SSL and MQTT handlers into a known state
  m_client.setProtocolVersion(QMqttClient::MQTT_5_0);
  m_sslConfiguration.setProtocol(QSsl::SecureProtocols);
  m_sslConfiguration.setPeerVerifyMode(QSslSocket::QueryPeer);
}

MQTT::Client &MQTT::Client::instance()
{
  static Client instance;
  return instance;
}

//------------------------------------------------------------------------------
// Member access functions
//------------------------------------------------------------------------------

quint8 MQTT::Client::mode() const
{
  return m_mode;
}

bool MQTT::Client::isConnected() const
{
  return m_client.state() == QMqttClient::Connected;
}

bool MQTT::Client::isPublisher() const
{
  return m_mode == 1;
}

bool MQTT::Client::isSubscriber() const
{
  return m_mode == 0;
}

bool MQTT::Client::cleanSession() const
{
  return m_client.cleanSession();
}

QString MQTT::Client::clientId() const
{
  return m_client.clientId();
}

QString MQTT::Client::hostname() const
{
  return m_client.hostname();
}

QString MQTT::Client::username() const
{
  return m_client.username();
}

QString MQTT::Client::password() const
{
  return m_client.password();
}

QString MQTT::Client::topicFilter() const
{
  return m_topicFilter;
}

quint8 MQTT::Client::willQoS() const
{
  return m_client.willQoS();
}

bool MQTT::Client::willRetain() const
{
  return m_client.willRetain();
}

QString MQTT::Client::willTopic() const
{
  return m_client.willTopic();
}

QString MQTT::Client::willMessage() const
{
  return m_client.willMessage();
}

quint16 MQTT::Client::port() const
{
  return m_client.port();
}

quint16 MQTT::Client::keepAlive() const
{
  return m_client.keepAlive();
}

bool MQTT::Client::autoKeepAlive() const
{
  return m_client.autoKeepAlive();
}

quint8 MQTT::Client::mqttVersion() const
{
  quint8 index = 0;
  for (auto i = m_mqttVersions.begin(); i != m_mqttVersions.end(); ++i)
  {
    if (i.value() == m_client.protocolVersion())
      break;

    ++index;
  }

  return index;
}

const QStringList &MQTT::Client::mqttVersions() const
{
  static QStringList list;
  if (list.isEmpty())
  {
    for (auto i = m_mqttVersions.begin(); i != m_mqttVersions.end(); ++i)
      list.append(i.key());
  }

  return list;
}

bool MQTT::Client::sslEnabled() const
{
  return m_sslEnabled;
}

quint8 MQTT::Client::sslProtocol() const
{
  quint8 index = 0;
  for (auto i = m_sslProtocols.begin(); i != m_sslProtocols.end(); ++i)
  {
    if (i.value() == m_sslConfiguration.protocol())
      break;

    ++index;
  }

  return index;
}

int MQTT::Client::peerVerifyDepth() const
{
  return m_sslConfiguration.peerVerifyDepth();
}

quint8 MQTT::Client::peerVerifyMode() const
{
  quint8 index = 0;
  for (auto i = m_peerVerifyModes.begin(); i != m_peerVerifyModes.end(); ++i)
  {
    if (i.value() == m_sslConfiguration.peerVerifyMode())
      break;

    ++index;
  }

  return index;
}

const QStringList &MQTT::Client::caCertificates() const
{
  static QStringList list;
  if (list.isEmpty())
  {
    list.append(tr("Use System Database"));
    list.append(tr("Load From Folder..."));
  }

  return list;
}

const QStringList &MQTT::Client::modes() const
{
  static QStringList list;
  if (list.isEmpty())
  {
    list.append(tr("MQTT Subscriber"));
    list.append(tr("MQTT Publisher"));
  }

  return list;
}

const QStringList &MQTT::Client::sslProtocols() const
{
  static QStringList list;
  if (list.isEmpty())
  {
    for (auto i = m_sslProtocols.begin(); i != m_sslProtocols.end(); ++i)
      list.append(i.key());
  }

  return list;
}

const QStringList &MQTT::Client::peerVerifyModes() const
{
  static QStringList list;
  if (list.isEmpty())
  {
    for (auto i = m_peerVerifyModes.begin(); i != m_peerVerifyModes.end(); ++i)
      list.append(i.key());
  }

  return list;
}

QMqttClient &MQTT::Client::client()
{
  return m_client;
}

//------------------------------------------------------------------------------
// Public slots
//------------------------------------------------------------------------------

void MQTT::Client::openConnection() {}
void MQTT::Client::closeConnection() {}

void MQTT::Client::toggleConnection()
{
  if (isConnected())
    closeConnection();
  else
    openConnection();
}

void MQTT::Client::setMode(const quint8 mode)
{
  Q_ASSERT(mode >= 0 && mode <= 1);
  m_mode = mode;
}

void MQTT::Client::setTopic(const QString &topic)
{
  m_topicFilter = topic;
  m_topicName.setName(topic);
}

void MQTT::Client::setClientId(const QString &id)
{
  m_client.setClientId(id);
}

void MQTT::Client::setHostname(const QString &hostname)
{
  m_client.setHostname(hostname);
}

void MQTT::Client::setUsername(const QString &username)
{
  m_client.setUsername(username);
}

void MQTT::Client::setPassword(const QString &password)
{
  m_client.setPassword(password);
}

void MQTT::Client::setCleanSession(const bool cleanSession)
{
  m_client.setCleanSession(cleanSession);
}

void MQTT::Client::setWillQoS(const quint8 qos)
{
  m_client.setWillQoS(qos);
}

void MQTT::Client::setWillRetain(const bool retain)
{
  m_client.setWillRetain(retain);
}

void MQTT::Client::setWillTopic(const QString &topic)
{
  m_client.setWillTopic(topic);
}

void MQTT::Client::setWillMessage(const QString &message)
{
  m_client.setWillMessage(message.toUtf8());
}

void MQTT::Client::setPort(const quint16 port)
{
  m_client.setPort(port);
}

void MQTT::Client::setKeepAlive(const quint16 keepAlive)
{
  m_client.setKeepAlive(keepAlive);
}

void MQTT::Client::setAutoKeepAlive(const bool keepAlive)
{
  m_client.setAutoKeepAlive(keepAlive);
}

void MQTT::Client::setMqttVersion(const quint8 version)
{
  quint8 index = 0;
  for (auto i = m_mqttVersions.begin(); i != m_mqttVersions.end(); ++i)
  {
    if (index == version)
    {
      m_client.setProtocolVersion(i.value());
      break;
    }

    ++index;
  }
}

void MQTT::Client::addCaCertificates()
{
  auto path = QFileDialog::getExistingDirectory(
      nullptr, tr("Select PEM Certificates Directory"),
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

  if (!path.isEmpty())
    m_sslConfiguration.addCaCertificates(path);
}

void MQTT::Client::setSslEnabled(const bool enabled)
{
  m_sslEnabled = enabled;
  Q_EMIT sslConfigurationChanged();
}

void MQTT::Client::setPeerVerifyDepth(const int depth)
{
  m_sslConfiguration.setPeerVerifyDepth(depth);
  Q_EMIT sslConfigurationChanged();
}

void MQTT::Client::setSslProtocol(const quint8 protocol)
{
  quint8 index = 0;
  for (auto i = m_sslProtocols.begin(); i != m_sslProtocols.end(); ++i)
  {
    if (index == protocol)
    {
      m_sslConfiguration.setProtocol(i.value());
      Q_EMIT sslConfigurationChanged();
      break;
    }

    ++index;
  }
}

void MQTT::Client::setPeerVerifyMode(const quint8 verifyMode)
{
  quint8 index = 0;
  for (auto i = m_peerVerifyModes.begin(); i != m_peerVerifyModes.end(); ++i)
  {
    if (index == verifyMode)
    {
      m_sslConfiguration.setPeerVerifyMode(i.value());
      Q_EMIT sslConfigurationChanged();
      break;
    }

    ++index;
  }
}

//------------------------------------------------------------------------------
// Private slots
//------------------------------------------------------------------------------

void MQTT::Client::publishMessage(const QByteArray &data)
{
  if (isConnected() && isPublisher() && !data.isEmpty())
    m_client.publish(m_topicName, data);
}

void MQTT::Client::onStateChanged(QMqttClient::ClientState state)
{
  Q_EMIT connectedChanged();
}

void MQTT::Client::onErrorChanged(QMqttClient::ClientError error)
{
  QString title;
  QString message;
  switch (error)
  {
    case QMqttClient::NoError:
      break;

    case QMqttClient::InvalidProtocolVersion:
      title = tr("Invalid MQTT Protocol Version");
      message = tr("The MQTT broker rejected the connection due to an "
                   "unsupported protocol version. Ensure that your client and "
                   "broker support the same protocol version.");
      break;

    case QMqttClient::IdRejected:
      title = tr("Client ID Rejected");
      message = tr("The broker rejected the client ID. It may be malformed, "
                   "too long, or already in use. Try using a different client "
                   "ID.");
      break;

    case QMqttClient::ServerUnavailable:
      title = tr("MQTT Server Unavailable");
      message = tr("The network connection was established, but the broker is "
                   "currently unavailable. Verify the broker status and try "
                   "again later.");
      break;

    case QMqttClient::BadUsernameOrPassword:
      title = tr("Authentication Error");
      message = tr("The username or password provided is incorrect or "
                   "malformed. Double-check your credentials and try again.");
      break;

    case QMqttClient::NotAuthorized:
      title = tr("Authorization Error");
      message = tr("The MQTT broker denied the connection due to insufficient "
                   "permissions. Ensure that your account has the necessary "
                   "access rights.");
      break;

    case QMqttClient::TransportInvalid:
      title = tr("Network or Transport Error");
      message = tr("A network or transport layer issue occurred, causing an "
                   "unexpected connection failure. "
                   "Check your network connection and broker settings.");
      break;

    case QMqttClient::ProtocolViolation:
      title = tr("MQTT Protocol Violation");
      message = tr("The client detected a violation of the MQTT protocol and "
                   "closed the connection. Check your MQTT implementation for "
                   "compliance.");
      break;

    case QMqttClient::UnknownError:
      title = tr("Unknown Error");
      message = tr("An unexpected error occurred. Check the logs for more "
                   "details or restart the application.");
      break;

    case QMqttClient::Mqtt5SpecificError:
      title = tr("MQTT 5 Error");
      message = tr("An MQTT protocol level 5 error occurred. "
                   "Check the broker logs or reason codes for more details.");
      break;

    default:
      break;
  }

  if (!title.isEmpty() && !message.isEmpty())
    Misc::Utilities::showMessageBox(title, message);
}

void MQTT::Client::onAuthenticationRequested(
    const QMqttAuthenticationProperties &p)
{
  // Ensure MQTT 5.0 is used for extended authentication
  if (m_client.protocolVersion() != QMqttClient::MQTT_5_0)
  {
    Misc::Utilities::showMessageBox(tr("Authentication Error"),
                                    tr("Extended authentication is required, "
                                       "but MQTT 5.0 is not enabled."));
    return;
  }

  // Retrieve authentication method requested by broker
  QString authMethod = p.authenticationMethod();
  if (authMethod.isEmpty())
    authMethod = tr("Unknown");

  // Alert user
  Misc::Utilities::showMessageBox(
      tr("MQTT Authentication Required"),
      tr("The MQTT broker requires authentication using method: \"%1\".\n\n"
         "Please provide the necessary credentials.")
          .arg(authMethod));

  // Get user name
  bool ok;
  const auto username
      = QInputDialog::getText(nullptr, tr("Enter MQTT Username"),
                              tr("Username:"), QLineEdit::Normal, "", &ok);
  if (!ok || username.isEmpty())
    return;

  // Get password
  const auto password
      = QInputDialog::getText(nullptr, tr("Enter MQTT Password"),
                              tr("Password:"), QLineEdit::Password, "", &ok);
  if (!ok || password.isEmpty())
    return;

  // Fill authentication properties
  QMqttAuthenticationProperties authProps;
  authProps.setAuthenticationMethod(authMethod);
  authProps.setAuthenticationData(
      QString("%1:%2").arg(username, password).toUtf8().toBase64());

  // Try authentication again
  m_client.authenticate(authProps);
}

void MQTT::Client::onAuthenticationFinished(
    const QMqttAuthenticationProperties &p)
{

  if (!p.reason().isEmpty())
  {
    Misc::Utilities::showMessageBox(
        tr("MQTT Authentication Failed"),
        tr("Authentication failed: %.").arg(p.reason()));
  }
}

#endif

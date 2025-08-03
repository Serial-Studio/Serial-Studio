/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include <QFile>
#include <QFileDialog>
#include <QInputDialog>

#include "IO/Manager.h"
#include "MQTT/Client.h"
#include "Misc/Utilities.h"
#include "Licensing/LemonSqueezy.h"

//------------------------------------------------------------------------------
// Constructor & singleton access functions
//------------------------------------------------------------------------------

MQTT::Client::Client()
  : m_publisher(false)
  , m_sslEnabled(false)
{
  // Set initial random client ID
  regenerateClientId();

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

  // Configure signals/slots between QtMQTT and this module
  connect(&m_client, &QMqttClient::stateChanged, this,
          &MQTT::Client::onStateChanged);
  connect(&m_client, &QMqttClient::errorChanged, this,
          &MQTT::Client::onErrorChanged);
  connect(&m_client, &QMqttClient::authenticationFinished, this,
          &MQTT::Client::onAuthenticationFinished);
  connect(&m_client, &QMqttClient::authenticationRequested, this,
          &MQTT::Client::onAuthenticationRequested);
  connect(&m_client, &QMqttClient::messageReceived, this,
          &MQTT::Client::onMessageReceived);

  // Disconnect from MQTT server when Serial Studio is deactivated
  connect(&Licensing::LemonSqueezy::instance(),
          &Licensing::LemonSqueezy::activatedChanged, this, [=, this] {
            if (isConnected() && !SerialStudio::activated())
              closeConnection();
          });

  // Set default parameters
  setPort(1883);
  setPeerVerifyDepth(10);
  setHostname("127.0.0.1");
}

/**
 * @brief Returns the singleton instance of the MQTT Client.
 * @return Reference to the shared MQTT::Client instance.
 */
MQTT::Client &MQTT::Client::instance()
{
  static Client instance;
  return instance;
}

//------------------------------------------------------------------------------
// Member access functions
//------------------------------------------------------------------------------

/**
 * @brief Returns the current client mode (publisher or subscriber).
 */
quint8 MQTT::Client::mode() const
{
  return m_mode;
}

/**
 * @brief Checks if the client is currently connected to the broker
 */
bool MQTT::Client::isConnected() const
{
  return m_client.state() == QMqttClient::Connected;
}

/**
 * @brief Returns true if the client is in publisher mode.
 */
bool MQTT::Client::isPublisher() const
{
  return m_mode == 1;
}

/**
 * @brief Returns true if the client is in subscriber mode.
 */
bool MQTT::Client::isSubscriber() const
{
  return m_mode == 0;
}

/**
 * @brief Returns whether the MQTT clean session flag is set.
 */
bool MQTT::Client::cleanSession() const
{
  return m_client.cleanSession();
}

/**
 * @brief Returns the current MQTT client ID.
 */
QString MQTT::Client::clientId() const
{
  return m_clientId;
}

/**
 * @brief Returns the hostname of the MQTT broker.
 */
QString MQTT::Client::hostname() const
{
  return m_client.hostname();
}

/**
 * @brief Returns the username used for authentication.
 */
QString MQTT::Client::username() const
{
  return m_client.username();
}

/**
 * @brief Returns the password used for authentication.
 */
QString MQTT::Client::password() const
{
  return m_client.password();
}

/**
 * @brief Returns the MQTT topic filter being used.
 */
QString MQTT::Client::topicFilter() const
{
  return m_topicFilter;
}

/**
 * @brief Returns the QoS level for the "Will" message.
 */
quint8 MQTT::Client::willQoS() const
{
  return m_client.willQoS();
}

/**
 * @brief Returns whether the "Will" message should be retained.
 */
bool MQTT::Client::willRetain() const
{
  return m_client.willRetain();
}

/**
 * @brief Returns the topic for the "Will" message.
 */
QString MQTT::Client::willTopic() const
{
  return m_client.willTopic();
}

/**
 * @brief Returns the message content of the "Will" message.
 */
QString MQTT::Client::willMessage() const
{
  return m_client.willMessage();
}

/**
 * @brief Returns the port number used to connect to the broker.
 */
quint16 MQTT::Client::port() const
{
  return m_client.port();
}

/**
 * @brief Returns the keep-alive interval in seconds.
 */
quint16 MQTT::Client::keepAlive() const
{
  return m_client.keepAlive();
}

/**
 * @brief Returns whether automatic keep-alive is enabled.
 */
bool MQTT::Client::autoKeepAlive() const
{
  return m_client.autoKeepAlive();
}

/**
 * @brief Returns the index of the selected MQTT protocol version.
 */
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

/**
 * @brief Returns the list of supported MQTT protocol version names.
 */
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

/**
 * @brief Returns true if SSL is enabled for connections.
 */
bool MQTT::Client::sslEnabled() const
{
  return m_sslEnabled;
}

/**
 * @brief Returns the index of the currently selected SSL protocol.
 */
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

/**
 * @brief Returns the peer verify depth used in SSL configuration.
 */
int MQTT::Client::peerVerifyDepth() const
{
  return m_sslConfiguration.peerVerifyDepth();
}

/**
 * @brief Returns the index of the selected peer verification mode.
 */
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

/**
 * @brief Returns the available certificate authority (CA) options.
 */
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

/**
 * @brief Returns the list of supported client modes (Publisher/Subscriber).
 */
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

/**
 * @brief Returns the list of supported SSL protocols.
 */
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

/**
 * @brief Returns the list of supported SSL peer verification modes.
 */
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

/**
 * @brief Provides direct access to the underlying QMqttClient instance.
 */
QMqttClient &MQTT::Client::client()
{
  return m_client;
}

//------------------------------------------------------------------------------
// Public slots
//------------------------------------------------------------------------------

/**
 * @brief Opens a connection to the MQTT broker.
 *
 * Configures the internal QMqttClient instance with the current parameters and
 * attempts to establish a connection to the broker. If SSL is enabled, sets the
 * appropriate SSL configuration.
 */
void MQTT::Client::openConnection()
{
  // Already connected, nothing to do
  if (isConnected())
    return;

  // Stop if Serial Studio is not activated
  if (!SerialStudio::activated())
  {
    Misc::Utilities::showMessageBox(
        tr("MQTT Feature Requires a Commercial License"),
        tr("Connecting to MQTT brokers is only available with a valid Serial "
           "Studio commercial license.\n\n"
           "To unlock this feature, please activate your license or visit the "
           "store."),
        QMessageBox::Warning);
    return;
  }

  // Verify that MQTT topic is set
  if (m_topicFilter.isEmpty())
  {
    if (isPublisher())
    {
      Misc::Utilities::showMessageBox(
          tr("Missing MQTT Topic"),
          tr("You must specify a topic before connecting as a publisher."),
          QMessageBox::Critical, tr("Configuration Error"));
      Q_EMIT highlightMqttTopicControl();
      return;
    }

    else
    {
      Misc::Utilities::showMessageBox(
          tr("MQTT Topic Not Set"),
          tr("You won't receive any messages until a topic is configured."),
          QMessageBox::Warning, tr("Configuration Warning"));
      Q_EMIT highlightMqttTopicControl();
    }
  }

  // Apply topic name if in publisher mode
  if (isPublisher())
  {
    m_topicName.setName(m_topicFilter);
    if (!m_topicName.isValid())
    {
      Misc::Utilities::showMessageBox(
          tr("Invalid MQTT Topic"),
          tr("The topic \"%1\" is not valid.").arg(m_topicFilter),
          QMessageBox::Critical, tr("Configuration Error"));
      Q_EMIT highlightMqttTopicControl();
      return;
    }
  }

  // Use random client ID if needed
  if (clientId().isEmpty())
    regenerateClientId();

  // Connect the client
  if (m_sslEnabled)
    m_client.connectToHostEncrypted(m_sslConfiguration);
  else
    m_client.connectToHost();
}

/**
 * @brief Closes the current connection to the broker.
 *
 * Gracefully disconnects the internal QMqttClient. This is a no-op if the
 * client is already disconnected.
 */
void MQTT::Client::closeConnection()
{
  if (!isConnected())
    return;

  m_client.disconnectFromHost();
}

/**
 * @brief Toggles the connection state (connect/disconnect).
 */
void MQTT::Client::toggleConnection()
{
  if (isConnected())
    closeConnection();
  else
    openConnection();
}

/**
 * @brief Regenerates and sets a new random MQTT client ID.
 *
 * This method creates a new client ID using a pseudo-random sequence of
 * lowercase alphanumeric characters. The generated ID is 16 characters long
 * and is assigned to the client using setClientId().
 *
 * This helps ensure uniqueness across MQTT client sessions and can be used
 * to recover or randomize identity without requiring external input.
 *
 * @note The character set used includes only lowercase letters and digits.
 */
void MQTT::Client::regenerateClientId()
{
  QString clientId;
  constexpr int length = 16;
  const QString charset = "abcdefghijklmnopqrstuvwxyz0123456789";
  for (int i = 0; i < length; ++i)
  {
    int index = QRandomGenerator::global()->bounded(charset.length());
    clientId.append(charset.at(index));
  }

  setClientId(clientId);
}

/**
 * @brief Sets the client mode (0=subscriber, 1=publisher).
 * @param mode Index of the desired mode.
 */
void MQTT::Client::setMode(const quint8 mode)
{
  Q_ASSERT(mode >= 0 && mode <= 1);
  m_mode = mode;
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the topic for subscription or publishing.
 * @param topic Topic name string.
 */
void MQTT::Client::setTopic(const QString &topic)
{
  m_topicFilter = topic;
  m_topicName.setName(topic);
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the MQTT client ID.
 */
void MQTT::Client::setClientId(const QString &id)
{
  m_clientId = id;
  m_client.setClientId(id);
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the broker hostname.
 */
void MQTT::Client::setHostname(const QString &hostname)
{
  m_client.setHostname(hostname);
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the username for authentication.
 */
void MQTT::Client::setUsername(const QString &username)
{
  m_client.setUsername(username);
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the password for authentication.
 */
void MQTT::Client::setPassword(const QString &password)
{
  m_client.setPassword(password);
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Enables or disables MQTT clean session flag.
 */
void MQTT::Client::setCleanSession(const bool cleanSession)
{
  m_client.setCleanSession(cleanSession);
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the QoS level for the "Will" message.
 */
void MQTT::Client::setWillQoS(const quint8 qos)
{
  m_client.setWillQoS(qos);
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Enables or disables message retention for the "Will" message.
 */
void MQTT::Client::setWillRetain(const bool retain)
{
  m_client.setWillRetain(retain);
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the topic for the "Will" message.
 */
void MQTT::Client::setWillTopic(const QString &topic)
{
  m_client.setWillTopic(topic);
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the message content for the "Will" message.
 */
void MQTT::Client::setWillMessage(const QString &message)
{
  m_client.setWillMessage(message.toUtf8());
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the connection port for the broker.
 */
void MQTT::Client::setPort(const quint16 port)
{
  m_client.setPort(port);
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the keep-alive interval.
 */
void MQTT::Client::setKeepAlive(const quint16 keepAlive)
{
  m_client.setKeepAlive(keepAlive);
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Enables or disables automatic keep-alive.
 */
void MQTT::Client::setAutoKeepAlive(const bool keepAlive)
{
  m_client.setAutoKeepAlive(keepAlive);
  Q_EMIT mqttConfigurationChanged();
}

/**
 * @brief Sets the MQTT protocol version by index.
 */
void MQTT::Client::setMqttVersion(const quint8 version)
{
  quint8 index = 0;
  for (auto i = m_mqttVersions.begin(); i != m_mqttVersions.end(); ++i)
  {
    if (index == version)
    {
      m_client.setProtocolVersion(i.value());
      Q_EMIT mqttConfigurationChanged();
      break;
    }

    ++index;
  }
}

/**
 * @brief Opens a file dialog to add CA certificates to the SSL config.
 */
void MQTT::Client::addCaCertificates()
{
  auto *dialog = new QFileDialog(
      nullptr, tr("Select PEM Certificates Directory"),
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

  dialog->setFileMode(QFileDialog::Directory);
  dialog->setOption(QFileDialog::ShowDirsOnly, true);
  dialog->setOption(QFileDialog::DontUseNativeDialog);

  connect(dialog, &QFileDialog::fileSelected, this,
          [this, dialog](const QString &path) {
            dialog->deleteLater();

            if (!path.isEmpty())
              m_sslConfiguration.addCaCertificates(path);
          });

  dialog->open();
}

/**
 * @brief Enables or disables SSL usage.
 */
void MQTT::Client::setSslEnabled(const bool enabled)
{
  m_sslEnabled = enabled;
  Q_EMIT sslConfigurationChanged();
}

/**
 * @brief Sets the peer verification depth in SSL config.
 */
void MQTT::Client::setPeerVerifyDepth(const int depth)
{
  m_sslConfiguration.setPeerVerifyDepth(depth);
  Q_EMIT sslConfigurationChanged();
}

/**
 * @brief Sets the SSL protocol by index.
 */
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

/**
 * @brief Sets the peer verification mode by index.
 */
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
// Hotpath functions
//------------------------------------------------------------------------------

/**
 * @brief Publishes a message to the broker if connected and in publisher mode.
 */
void MQTT::Client::hotpathTxFrame(const QByteArray &data)
{
  if (isConnected() && isPublisher() && m_topicName.isValid()
      && SerialStudio::activated())
    m_client.publish(m_topicName, data);
}

//------------------------------------------------------------------------------
// Private slots
//------------------------------------------------------------------------------

/**
 * @brief Handles changes in the client's connection state.
 *
 * Emits the connectedChanged() signal whenever the connection state changes.
 * If the client transitions to the Connected state and is in subscriber mode,
 * this will attempt to subscribe to the configured topic filter with QoS 0.
 * Displays an error message if the subscription fails.
 *
 * @param state The new connection state of the QMqttClient.
 */
void MQTT::Client::onStateChanged(QMqttClient::ClientState state)
{
  Q_EMIT connectedChanged();
  if (state == QMqttClient::Connected && isSubscriber()
      && !m_topicFilter.isEmpty())
  {
    QMqttTopicFilter filter;
    filter.setFilter(m_topicFilter);

    auto *sub = m_client.subscribe(filter, 0);
    if (!sub || sub->state() == QMqttSubscription::Error)
    {
      Misc::Utilities::showMessageBox(
          tr("Subscription Error"),
          tr("Failed to subscribe to topic \"%1\".").arg(m_topicFilter),
          QMessageBox::Critical);
    }
  }
}

/**
 * @brief Responds to error changes by showing a user-friendly message.
 */
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
    Misc::Utilities::showMessageBox(title, message, QMessageBox::Critical);
}

/**
 * @brief Handles the result of an authentication attempt.
 */
void MQTT::Client::onAuthenticationFinished(
    const QMqttAuthenticationProperties &p)
{

  if (!p.reason().isEmpty())
  {
    Misc::Utilities::showMessageBox(
        tr("MQTT Authentication Failed"),
        tr("Authentication failed: %.").arg(p.reason()), QMessageBox::Warning);
  }
}

/**
 * @brief Handles extended authentication requests from the broker.
 */
void MQTT::Client::onAuthenticationRequested(
    const QMqttAuthenticationProperties &p)
{
  // Ensure MQTT 5.0 is used for extended authentication
  if (m_client.protocolVersion() != QMqttClient::MQTT_5_0)
  {
    Misc::Utilities::showMessageBox(tr("Authentication Error"),
                                    tr("Extended authentication is required, "
                                       "but MQTT 5.0 is not enabled."),
                                    QMessageBox::Warning);
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
          .arg(authMethod),
      QMessageBox::Information);

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

/**
 * @brief Handles incoming MQTT messages.
 *
 * This slot is called when the client receives a message from the broker.
 * It processes the message only if:
 * - The message payload is non-empty
 * - The client is in subscriber mode
 * - The topic matches the currently configured subscription topic
 *
 * If all conditions are met, the message payload is passed to IO::Manager
 * for processing via a queued Qt method invocation.
 *
 * @param message The received MQTT message payload.
 * @param topic The topic associated with the received message.
 */
void MQTT::Client::onMessageReceived(const QByteArray &message,
                                     const QMqttTopicName &topic)
{
  // Stop if Serial Studio is not activated
  if (!SerialStudio::activated())
    return;

  // Only process if data is not empty
  if (!message.isEmpty())
  {
    // Ignore if client mode is not set to suscriber
    if (!isSubscriber())
      return;

    // Ignore if topic is not equal to current topic
    if (m_topicName != topic)
      return;

    // Let IO manager process incoming data
    IO::Manager::instance().processPayload(message);
  }
}

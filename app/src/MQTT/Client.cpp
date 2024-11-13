/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <QFile>
#include <QFileDialog>

#include "IO/Manager.h"
#include "MQTT/Client.h"
#include "Misc/Utilities.h"

//----------------------------------------------------------------------------
// Suppress deprecated warnings
//----------------------------------------------------------------------------

#if defined(__GNUC__) || defined(__clang__)
#  define DISABLE_DEPRECATED_WARNING                                           \
    _Pragma("GCC diagnostic push")                                             \
        _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#  define ENABLE_DEPRECATED_WARNING _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#  define DISABLE_DEPRECATED_WARNING                                           \
    __pragma(warning(push)) __pragma(warning(disable : 4996))
#  define ENABLE_DEPRECATED_WARNING __pragma(warning(pop))
#else
#  define DISABLE_DEPRECATED_WARNING
#  define ENABLE_DEPRECATED_WARNING
#endif

/**
 * Constructor function
 */
MQTT::Client::Client()
  : m_topic("")
  , m_lookupActive(false)
  , m_sentMessages(0)
  , m_clientMode(MQTTClientMode::ClientPublisher)
  , m_client(nullptr)
{
  // Configure new client
  regenerateClient();

  // Send data periodically & reset statistics when disconnected/connected to a
  connect(&IO::Manager::instance(), &IO::Manager::frameReceived, this,
          &MQTT::Client::sendFrame);
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this,
          &MQTT::Client::resetStatistics);

  // Disconenct from current IO connection when MQTT client is subscribed
  connect(this, &MQTT::Client::connectedChanged, this, [=] {
    if (isSubscribed())
    {
      if (IO::Manager::instance().connected())
        IO::Manager::instance().disconnectDevice();
    }
  });
}

/**
 * Destructor function
 */
MQTT::Client::~Client()
{
  delete m_client;
}

/**
 * Returns a pointer to the only instance of this class
 */
MQTT::Client &MQTT::Client::instance()
{
  static Client singleton;
  return singleton;
}

/**
 * Returns the quality-of-service option, available values:
 * - 0: at most once
 * - 1: at least once
 * - 2: exactly once
 */
quint8 MQTT::Client::qos() const
{
  Q_ASSERT(m_client);
  return m_client->willQos();
}

/**
 * Returns @c true if the retain flag is enabled
 */
bool MQTT::Client::retain() const
{
  Q_ASSERT(m_client);
  return m_client->willRetain();
}

/**
 * Returns the TCP port number used for the MQTT connection
 */
quint16 MQTT::Client::port() const
{
  Q_ASSERT(m_client);
  return m_client->port();
}

/**
 * Returns the MQTT topic used
 */
QString MQTT::Client::topic() const
{
  return m_topic;
}

/**
 * Returns the selected SSL/TLS protocol index
 */
int MQTT::Client::sslProtocol() const
{
  return m_sslProtocol;
}

/**
 * Returns the index of the MQTT version, corresponding to the list returned by
 * the
 * @c mqttVersions() function.
 */
int MQTT::Client::mqttVersion() const
{
  Q_ASSERT(m_client);

  switch (m_client->version())
  {
    case QMQTT::V3_1_0:
      return 0;
      break;
    case QMQTT::V3_1_1:
      return 1;
      break;
    default:
      return -1;
      break;
  }
}

/**
 * Returns @c true if SSL/TLS is enabled
 */
bool MQTT::Client::sslEnabled() const
{
  return m_sslEnabled;
}

/**
 * Returns the client mode, which can have the following values:
 * - Publisher
 * - Subscriber
 */
int MQTT::Client::clientMode() const
{
  return m_clientMode;
}

/**
 * Returns the MQTT username
 */
QString MQTT::Client::username() const
{
  Q_ASSERT(m_client);
  return m_client->username();
}

/**
 * Returns the MQTT password
 */
QString MQTT::Client::password() const
{
  Q_ASSERT(m_client);
  return QString::fromUtf8(m_client->password());
}

/**
 * Returns the MQTT client ID
 */
QString MQTT::Client::clientId() const
{
  Q_ASSERT(m_client);
  return m_client->clientId();
}

/**
 * Returns the IP address of the MQTT broker/server
 */
QString MQTT::Client::host() const
{
  Q_ASSERT(m_client);
  return m_client->hostName();
}

/**
 * Returns the keep-alive timeout interval used by the MQTT client.
 */
quint16 MQTT::Client::keepAlive() const
{
  Q_ASSERT(m_client);
  return m_client->keepAlive();
}

/**
 * Returns @c true if the MQTT module is currently performing a DNS lookup of
 * the MQTT broker/server domain.
 */
bool MQTT::Client::lookupActive() const
{
  return m_lookupActive;
}

/**
 * Returns @c true if the MQTT module is connected to the broker, the topic is
 * not empty and the client is configured to act as an MQTT subscriber.
 */
bool MQTT::Client::isSubscribed() const
{
  return isConnectedToHost() && !topic().isEmpty()
         && clientMode() == ClientSubscriber;
}

/**
 * Returns @c true if the MQTT module is connected to a MQTT broker/server.
 */
bool MQTT::Client::isConnectedToHost() const
{
  Q_ASSERT(m_client);
  return m_client->isConnectedToHost();
}

/**
 * Returns a list with the available quality-of-service modes.
 */
QStringList MQTT::Client::qosLevels() const
{
  return QStringList{tr("0: At most once"), tr("1: At least once"),
                     tr("2: Exactly once")};
}

/**
 * Returns a list with the available client operation modes.
 */
QStringList MQTT::Client::clientModes() const
{
  return QStringList{tr("Publisher"), tr("Subscriber")};
}

/**
 * Returns a list with the supported MQTT versions
 */
QStringList MQTT::Client::mqttVersions() const
{
  return QStringList{"MQTT 3.1.0", "MQTT 3.1.1"};
}

/**
 * Returns a list with the supported SSL/TLS protocols
 */
QStringList MQTT::Client::sslProtocols() const
{
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
  return QStringList{tr("System default"), "TLS v1.0", "TLS v1.1", "TLS v1.2"};
#else
  return QStringList{
      tr("System default"),  "TLS v1.0",  "TLS v1.1",  "TLS v1.2",
      "TLS v1.3 (or later)", "DTLS v1.0", "DTLS v1.2", "DTLS v1.2 (or later)"};
#endif
}

/**
 * Returns the path of the currently loaded *.ca file
 */
QString MQTT::Client::caFilePath() const
{
  return m_caFilePath;
}

/**
 * Prompts the user to select a *.ca file and loads the certificate
 * into the SSL configuration.
 */
void MQTT::Client::loadCaFile()
{
  // Prompt user to select a CA file
  // clang-format off
  auto path = QFileDialog::getOpenFileName(nullptr,
                                           tr("Select CA file"),
                                           QDir::homePath());
  // clang-format on

  // Try to load the *.ca file
  loadCaFile(path);
}

/**
 * Tries to establish a TCP connection with the MQTT broker/server.
 */
void MQTT::Client::connectToHost()
{
  Q_ASSERT(m_client);
  m_client->connectToHost();
}

/**
 * Connects/disconnects the application from the current MQTT broker. This
 * function is used as a convenience for the connect/disconnect button.
 */
void MQTT::Client::toggleConnection()
{
  if (isConnectedToHost())
    disconnectFromHost();
  else
    connectToHost();
}

/**
 * Disconnects from the MQTT broker/server
 */
void MQTT::Client::disconnectFromHost()
{
  Q_ASSERT(m_client);
  m_client->disconnectFromHost();
}

/**
 * Changes the quality of service level of the MQTT client.
 */
void MQTT::Client::setQos(const quint8 qos)
{
  Q_ASSERT(m_client);
  m_client->setWillQos(qos);
  Q_EMIT qosChanged();
}

/**
 * Performs a DNS lookup for the given @a host name
 */
void MQTT::Client::lookup(const QString &host)
{
  m_lookupActive = true;
  Q_EMIT lookupActiveChanged();
  QHostInfo::lookupHost(host.simplified(), this, &MQTT::Client::lookupFinished);
}

/**
 * Changes the TCP port number used for the MQTT communications.
 */
void MQTT::Client::setPort(const quint16 port)
{
  Q_ASSERT(m_client);
  m_client->setPort(port);
  Q_EMIT portChanged();
}

/**
 * Changes the IP address of the MQTT broker/host
 */
void MQTT::Client::setHost(const QString &host)
{
  Q_ASSERT(m_client);
  m_client->setHostName(host);
  Q_EMIT hostChanged();
}

/**
 * If set to @c true, the @c retain flag shall be appended to the MQTT message
 * so that new clients connecting to the broker will immediately receive the
 * last "good" message.
 */
void MQTT::Client::setRetain(const bool retain)
{
  Q_ASSERT(m_client);
  m_client->setWillRetain(retain);
  Q_EMIT retainChanged();
}

/**
 * Changes the operation mode of the MQTT client. Possible values are:
 * - Publisher
 * - Subscriber
 */
void MQTT::Client::setClientMode(const int mode)
{
  m_clientMode = static_cast<MQTTClientMode>(mode);
  Q_EMIT clientModeChanged();
}

/**
 * Changes the MQTT topic used by the client.
 */
void MQTT::Client::setTopic(const QString &topic)
{
  m_topic = topic;
  Q_EMIT topicChanged();
}

/**
 * Reads the CA file in the given @a path and loads it into the
 * SSL configuration handler for the MQTT connection.
 */
void MQTT::Client::loadCaFile(const QString &path)
{
  // Save *.ca file path
  m_caFilePath = path;
  Q_EMIT caFilePathChanged();

  // Empty path, abort
  if (path.isEmpty())
    return;

  // Try to read file contents
  QByteArray data;
  QFile file(path);
  if (file.open(QFile::ReadOnly))
  {
    data = file.readAll();
    file.close();
  }

  // Read error, alert user
  else
  {
    Misc::Utilities::showMessageBox(tr("Cannot open CA file!"),
                                    file.errorString());
    file.close();
    return;
  }

  // Load certificate into SSL configuration
  m_sslConfiguration.setCaCertificates(QSslCertificate::fromData(data));
  regenerateClient();
}

/**
 * Changes the SSL protocol version to use for the MQTT connection.
 */
void MQTT::Client::setSslProtocol(const int index)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
  switch (index)
  {
    case 0:
      m_sslConfiguration.setProtocol(QSsl::SecureProtocols);
      break;
    case 1:
      m_sslConfiguration.setProtocol(QSsl::TlsV1_0);
      break;
    case 2:
      m_sslConfiguration.setProtocol(QSsl::TlsV1_1);
      break;
    case 3:
      m_sslConfiguration.setProtocol(QSsl::TlsV1_2);
      break;
    default:
      break;
  }
#else
  switch (index)
  {
    case 0:
      m_sslConfiguration.setProtocol(QSsl::SecureProtocols);
      break;
    case 1:
      DISABLE_DEPRECATED_WARNING
      m_sslConfiguration.setProtocol(QSsl::TlsV1_0);
      ENABLE_DEPRECATED_WARNING
      break;
    case 2:
      DISABLE_DEPRECATED_WARNING
      m_sslConfiguration.setProtocol(QSsl::TlsV1_1);
      ENABLE_DEPRECATED_WARNING
      break;
    case 3:
      m_sslConfiguration.setProtocol(QSsl::TlsV1_2);
      break;
    case 4:
      m_sslConfiguration.setProtocol(QSsl::TlsV1_3OrLater);
      break;
    case 5:
      DISABLE_DEPRECATED_WARNING
      m_sslConfiguration.setProtocol(QSsl::DtlsV1_0);
      ENABLE_DEPRECATED_WARNING
      break;
    case 6:
      m_sslConfiguration.setProtocol(QSsl::DtlsV1_2);
      break;
    case 7:
      m_sslConfiguration.setProtocol(QSsl::DtlsV1_2OrLater);
      break;
    default:
      break;
  }
#endif

  regenerateClient();
  Q_EMIT sslProtocolChanged();
}

/**
 * Enables/disables SSL/TLS communications with the MQTT broker
 */
void MQTT::Client::setSslEnabled(const bool enabled)
{
  m_sslEnabled = enabled;
  regenerateClient();
  Q_EMIT sslEnabledChanged();
}

/**
 * Changes the username used to connect to the MQTT broker/server
 */
void MQTT::Client::setUsername(const QString &username)
{
  Q_ASSERT(m_client);
  m_client->setUsername(username);
  Q_EMIT usernameChanged();
}

/**
 * Changes the password used to connect to the MQTT broker/server
 */
void MQTT::Client::setPassword(const QString &password)
{
  Q_ASSERT(m_client);
  m_client->setPassword(password.toUtf8());
  Q_EMIT passwordChanged();
}

/**
 * Changes the client ID used for the MQTT connection.
 */
void MQTT::Client::setClientId(const QString &clientId)
{
  Q_ASSERT(m_client);
  m_client->setClientId(clientId);
  Q_EMIT clientIdChanged();
}

/**
 * Sets the maximum time interval that is permitted to elapse between the point
 * at which the Client finishes transmitting one Control Packet and the point it
 * starts sending the next packet.
 */
void MQTT::Client::setKeepAlive(const quint16 keepAlive)
{
  Q_ASSERT(m_client);
  m_client->setKeepAlive(keepAlive);
  Q_EMIT keepAliveChanged();
}

/**
 * Changes the MQTT version used to connect to the MQTT broker/server
 */
void MQTT::Client::setMqttVersion(const int versionIndex)
{
  Q_ASSERT(m_client);

  switch (versionIndex)
  {
    case 0:
      m_client->setVersion(QMQTT::V3_1_0);
      break;
    case 1:
      m_client->setVersion(QMQTT::V3_1_1);
      break;
    default:
      break;
  }

  Q_EMIT mqttVersionChanged();
}

/**
 * Clears the JSON frames & sets the sent messages to 0
 */
void MQTT::Client::resetStatistics()
{
  m_sentMessages = 0;
}

/**
 * Subscribe/unsubscripe to the set MQTT topic when the connection state is
 * changed.
 */
void MQTT::Client::onConnectedChanged()
{
  Q_ASSERT(m_client);

  if (isConnectedToHost())
    m_client->subscribe(topic());
  else
    m_client->unsubscribe(topic());
}

/**
 * @brief Sends a data frame to the MQTT broker
 *
 * @param frame The frame containing the data to be sent.
 */
void MQTT::Client::sendFrame(const QByteArray &frame)
{
  Q_ASSERT(m_client);

  // Ignore if device is not connected
  if (!IO::Manager::instance().connected())
    return;

  // Ignore if client is not connected
  else if (!isConnectedToHost())
    return;

  // Ignore if mode is not set to publisher
  else if (clientMode() != ClientPublisher)
    return;

  // Create & send MQTT message
  if (!frame.isEmpty())
  {
    QMQTT::Message message(m_sentMessages, topic(), frame);
    m_client->publish(message);
    ++m_sentMessages;
  }
}

/**
 * Sets the host IP address when the lookup finishes.
 * If the lookup fails, the error code/string shall be shown to the user in a
 * messagebox.
 */
void MQTT::Client::lookupFinished(const QHostInfo &info)
{
  m_lookupActive = false;
  Q_EMIT lookupActiveChanged();

  if (info.error() == QHostInfo::NoError)
  {
    auto addresses = info.addresses();
    if (addresses.count() >= 1)
    {
      setHost(addresses.first().toString());
      return;
    }
  }

  Misc::Utilities::showMessageBox(tr("IP address lookup error"),
                                  info.errorString());
}

/**
 * Displays any MQTT-related error with a GUI message-box
 */
void MQTT::Client::onError(const QMQTT::ClientError error)
{
  QString str;

  switch (error)
  {
    case QMQTT::UnknownError:
      str = tr("Unknown error");
      break;
    case QMQTT::SocketConnectionRefusedError:
      str = tr("Connection refused");
      break;
    case QMQTT::SocketRemoteHostClosedError:
      str = tr("Remote host closed the connection");
      break;
    case QMQTT::SocketHostNotFoundError:
      str = tr("Host not found");
      break;
    case QMQTT::SocketAccessError:
      str = tr("Socket access error");
      break;
    case QMQTT::SocketResourceError:
      str = tr("Socket resource error");
      break;
    case QMQTT::SocketTimeoutError:
      str = tr("Socket timeout");
      break;
    case QMQTT::SocketDatagramTooLargeError:
      str = tr("Socket datagram too large");
      break;
    case QMQTT::SocketNetworkError:
      str = tr("Network error");
      break;
    case QMQTT::SocketAddressInUseError:
      str = tr("Address in use");
      break;
    case QMQTT::SocketAddressNotAvailableError:
      str = tr("Address not available");
      break;
    case QMQTT::SocketUnsupportedSocketOperationError:
      str = tr("Unsupported socket operation");
      break;
    case QMQTT::SocketUnfinishedSocketOperationError:
      str = tr("Unfinished socket operation");
      break;
    case QMQTT::SocketProxyAuthenticationRequiredError:
      str = tr("Proxy authentication required");
      break;
    case QMQTT::SocketSslHandshakeFailedError:
      str = tr("SSL handshake failed");
      break;
    case QMQTT::SocketProxyConnectionRefusedError:
      str = tr("Proxy connection refused");
      break;
    case QMQTT::SocketProxyConnectionClosedError:
      str = tr("Proxy connection closed");
      break;
    case QMQTT::SocketProxyConnectionTimeoutError:
      str = tr("Proxy connection timeout");
      break;
    case QMQTT::SocketProxyNotFoundError:
      str = tr("Proxy not found");
      break;
    case QMQTT::SocketProxyProtocolError:
      str = tr("Proxy protocol error");
      break;
    case QMQTT::SocketOperationError:
      str = tr("Operation error");
      break;
    case QMQTT::SocketSslInternalError:
      str = tr("SSL internal error");
      break;
    case QMQTT::SocketSslInvalidUserDataError:
      str = tr("Invalid SSL user data");
      break;
    case QMQTT::SocketTemporaryError:
      str = tr("Socket temprary error");
      break;
    case QMQTT::MqttUnacceptableProtocolVersionError:
      str = tr("Unacceptable MQTT protocol");
      break;
    case QMQTT::MqttIdentifierRejectedError:
      str = tr("MQTT identifier rejected");
      break;
    case QMQTT::MqttServerUnavailableError:
      str = tr("MQTT server unavailable");
      break;
    case QMQTT::MqttBadUserNameOrPasswordError:
      str = tr("Bad MQTT username or password");
      break;
    case QMQTT::MqttNotAuthorizedError:
      str = tr("MQTT authorization error");
      break;
    case QMQTT::MqttNoPingResponse:
      str = tr("MQTT no ping response");
      break;
    default:
      str = "";
      break;
  }

  if (!str.isEmpty())
    Misc::Utilities::showMessageBox(tr("MQTT client error"), str);
}

/**
 * Displays the SSL errors that occur and allows the user to decide if he/she
 * wants to ignore those errors.
 */
void MQTT::Client::onSslErrors(const QList<QSslError> &errors)
{
  Q_ASSERT(m_client);

  Q_FOREACH (auto error, errors)
  {
    auto ret = Misc::Utilities::showMessageBox(
        tr("MQTT client SSL/TLS error, ignore?"), error.errorString(),
        qApp->applicationDisplayName(),
        QMessageBox::Ignore | QMessageBox::Abort);

    if (ret == QMessageBox::Abort)
    {
      disconnectFromHost();
      abort();
    }
  }

  m_client->ignoreSslErrors();
}

/**
 * Reads the given MQTT @a message and instructs the @c IO::Manager module to
 * process received data directly.
 */
void MQTT::Client::onMessageReceived(const QMQTT::Message &message)
{
  // Ignore if client mode is not set to suscriber
  if (clientMode() != ClientSubscriber)
    return;

  // Get message data
  auto mtopic = message.topic();
  auto mpayld = message.payload();

  // Ignore if topic is not equal to current topic
  if (topic() != mtopic)
    return;

  // Let IO manager process incoming data
  QMetaObject::invokeMethod(
      this, [=] { IO::Manager::instance().processPayload(mpayld); },
      Qt::QueuedConnection);
}

/**
 * Creates a new MQTT client instance, this approach is required in order to
 * allow the MQTT module to support both non-encrypted and TLS connections.
 */
void MQTT::Client::regenerateClient()
{
  // Init. default MQTT configuration
  quint8 qos = 0;
  QString user = "";
  bool retain = false;
  QString password = "";
  quint16 keepAlive = 60;
  quint16 port = defaultPort();
  QString host = defaultHost();
  QMQTT::MQTTVersion version = QMQTT::V3_1_1;
  QString clientId = QStringLiteral("SerialStudio");

  // There is an existing client, copy its configuration and delete it from
  // memory
  if (m_client)
  {
    port = m_client->port();

    qos = m_client->willQos();
    user = m_client->username();
    version = m_client->version();
    clientId = m_client->clientId();
    retain = m_client->willRetain();
    keepAlive = m_client->keepAlive();
    host = m_client->host().toString();
    password = QString::fromUtf8(m_client->password());

    disconnect(m_client, &QMQTT::Client::error, nullptr, 0);
    disconnect(m_client, &QMQTT::Client::received, nullptr, 0);
    disconnect(m_client, &QMQTT::Client::connected, nullptr, 0);
    disconnect(m_client, &QMQTT::Client::sslErrors, nullptr, 0);
    disconnect(m_client, &QMQTT::Client::disconnected, nullptr, 0);

    m_client->disconnectFromHost();
    delete m_client;
  }

  // Configure MQTT client depending on SSL/TLS configuration
  if (sslEnabled())
    m_client = new QMQTT::Client(host, port, m_sslConfiguration);
  else
    m_client = new QMQTT::Client(QHostAddress(host), port);

  // Set client ID
  m_client->setClientId(qApp->applicationDisplayName());

  // Set MQTT client options
  m_client->setWillQos(qos);
  m_client->setUsername(user);
  m_client->setVersion(version);
  m_client->setClientId(clientId);
  m_client->setWillRetain(retain);
  m_client->setKeepAlive(keepAlive);
  m_client->setPassword(password.toUtf8());

  // Connect signals/slots
  connect(m_client, &QMQTT::Client::error, this, &MQTT::Client::onError);
  connect(m_client, &QMQTT::Client::sslErrors, this,
          &MQTT::Client::onSslErrors);
  connect(m_client, &QMQTT::Client::received, this,
          &MQTT::Client::onMessageReceived);
  connect(m_client, &QMQTT::Client::connected, this,
          &MQTT::Client::connectedChanged);
  connect(m_client, &QMQTT::Client::connected, this,
          &MQTT::Client::onConnectedChanged);
  connect(m_client, &QMQTT::Client::disconnected, this,
          &MQTT::Client::connectedChanged);
  connect(m_client, &QMQTT::Client::disconnected, this,
          &MQTT::Client::onConnectedChanged);
}

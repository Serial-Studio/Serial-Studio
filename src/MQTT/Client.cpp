/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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

#include "Client.h"

#include <IO/Manager.h>
#include <Misc/Utilities.h>
#include <Misc/TimerEvents.h>

namespace MQTT
{
static Client *CLIENT = nullptr;

/**
 * Constructor function
 */
Client::Client()
    : m_topic("")
    , m_lookupActive(false)
    , m_sentMessages(0)
    , m_clientMode(MQTTClientMode::ClientPublisher)
{
    // MQTT signals/slots
    connect(&m_client, &QMQTT::Client::error, this, &Client::onError);
    connect(&m_client, &QMQTT::Client::received, this, &Client::onMessageReceived);
    connect(&m_client, &QMQTT::Client::connected, this, &Client::connectedChanged);
    connect(&m_client, &QMQTT::Client::disconnected, this, &Client::connectedChanged);
    connect(&m_client, &QMQTT::Client::connected, this, &Client::onConnectedChanged);
    connect(&m_client, &QMQTT::Client::disconnected, this, &Client::onConnectedChanged);

    // Send data periodically & reset statistics when disconnected/connected to a device
    auto io = IO::Manager::getInstance();
    auto te = Misc::TimerEvents::getInstance();
    connect(te, &Misc::TimerEvents::lowFreqTimeout, this, &Client::sendData);
    connect(io, &IO::Manager::connectedChanged, this, &Client::resetStatistics);
    connect(io, &IO::Manager::frameReceived, this, &Client::onFrameReceived);

    // Set default port/host
    setPort(defaultPort());
    setHost(defaultHost());
}

/**
 * Destructor function
 */
Client::~Client()
{
    disconnectFromHost();
}

/**
 * Returns a pointer to the only instance of this class
 */
Client *Client::getInstance()
{
    if (!CLIENT)
        CLIENT = new Client;

    return CLIENT;
}

/**
 * Returns the TCP port number used for the MQTT connection
 */
quint16 Client::port() const
{
    return m_client.port();
}

/**
 * Returns the MQTT topic used
 */
QString Client::topic() const
{
    return m_topic;
}

/**
 * Returns the index of the MQTT version, corresponding to the list returned by the
 * @c mqttVersions() function.
 */
int Client::mqttVersion() const
{
    switch (m_client.version())
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
 * Returns the client mode, which can have the following values:
 * - Publisher
 * - Subscriber
 */
int Client::clientMode() const
{
    return m_clientMode;
}

/**
 * Returns the MQTT username
 */
QString Client::username() const
{
    return m_client.username();
}

/**
 * Returns the MQTT password
 */
QString Client::password() const
{
    return QString::fromUtf8(m_client.password());
}

/**
 * Returns the IP address of the MQTT broker/server
 */
QString Client::host() const
{
    return m_client.hostName();
}

/**
 * Returns @c true if the MQTT module is currently performing a DNS lookup of the MQTT
 * broker/server domain.
 */
bool Client::lookupActive() const
{
    return m_lookupActive;
}

/**
 * Returns @c true if the MQTT module is connected to the broker, the topic is not empty
 * and the client is configured to act as an MQTT subscriber.
 */
bool Client::isSubscribed() const
{
    return isConnectedToHost() && !topic().isEmpty() && clientMode() == ClientSubscriber;
}

/**
 * Returns @c true if the MQTT module is connected to a MQTT broker/server.
 */
bool Client::isConnectedToHost() const
{
    return m_client.isConnectedToHost();
}

/**
 * Returns a list with the available client operation modes.
 */
QVector<QString> Client::clientModes() const
{
    return QVector<QString> { tr("Publisher"), tr("Subscriber") };
}

/**
 * Returns a list with the supported MQTT versions
 */
QVector<QString> Client::mqttVersions() const
{
    return QVector<QString> { "MQTT 3.1.0", "MQTT 3.1.1" };
}

/**
 * Tries to establish a TCP connection with the MQTT broker/server.
 */
void Client::connectToHost()
{
    m_client.connectToHost();
}

/**
 * Connects/disconnects the application from the current MQTT broker. This function is
 * used as a convenience for the connect/disconnect button.
 */
void Client::toggleConnection()
{
    if (isConnectedToHost())
        disconnectFromHost();
    else
        connectToHost();
}

/**
 * Disconnects from the MQTT broker/server
 */
void Client::disconnectFromHost()
{
    m_client.disconnectFromHost();
}

/**
 * Performs a DNS lookup for the given @a host name
 */
void Client::lookup(const QString &host)
{
    m_lookupActive = true;
    emit lookupActiveChanged();
    QHostInfo::lookupHost(host.simplified(), this, &Client::lookupFinished);
}

/**
 * Changes the TCP port number used for the MQTT communications.
 */
void Client::setPort(const quint16 port)
{
    m_client.setPort(port);
    emit portChanged();
}

/**
 * Changes the IP address of the MQTT broker/host
 */
void Client::setHost(const QString &host)
{
    m_client.setHostName(host);
    emit hostChanged();
}

/**
 * Changes the operation mode of the MQTT client. Possible values are:
 * - Publisher
 * - Subscriber
 */
void Client::setClientMode(const int mode)
{
    m_clientMode = static_cast<MQTTClientMode>(mode);
    emit clientModeChanged();
}

/**
 * Changes the MQTT topic used by the client.
 */
void Client::setTopic(const QString &topic)
{
    m_topic = topic;
    emit topicChanged();
}

/**
 * Changes the username used to connect to the MQTT broker/server
 */
void Client::setUsername(const QString &username)
{
    m_client.setUsername(username);
    emit usernameChanged();
}

/**
 * Changes the password used to connect to the MQTT broker/server
 */
void Client::setPassword(const QString &password)
{
    m_client.setPassword(password.toUtf8());
    emit passwordChanged();
}

/**
 * Changes the MQTT version used to connect to the MQTT broker/server
 */
void Client::setMqttVersion(const int versionIndex)
{
    switch (versionIndex)
    {
        case 0:
            m_client.setVersion(QMQTT::V3_1_0);
            break;
        case 1:
            m_client.setVersion(QMQTT::V3_1_1);
            break;
        default:
            break;
    }

    emit mqttVersionChanged();
}

/**
 * Publishes all the received data to the MQTT broker
 */
void Client::sendData()
{
    // Create data byte array
    QByteArray data;
    for (int i = 0; i < m_frames.count(); ++i)
    {
        data.append(m_frames.at(i));
        data.append("\n");
    }

    // Create & send MQTT message
    if (!data.isEmpty())
    {
        QMQTT::Message message(m_sentMessages, topic(), data);
        m_client.publish(message);
        ++m_sentMessages;
    }

    // Clear frame list
    m_frames.clear();
}

/**
 * Clears the JSON frames & sets the sent messages to 0
 */
void Client::resetStatistics()
{
    m_sentMessages = 0;
    m_frames.clear();
}

/**
 * Subscribe/unsubscripe to the set MQTT topic when the connection state is changed.
 */
void Client::onConnectedChanged()
{
    if (isConnectedToHost())
        m_client.subscribe(topic());
    else
        m_client.unsubscribe(topic());
}

/**
 * Sets the host IP address when the lookup finishes.
 * If the lookup fails, the error code/string shall be shown to the user in a messagebox.
 */
void Client::lookupFinished(const QHostInfo &info)
{
    m_lookupActive = false;
    emit lookupActiveChanged();

    if (info.error() == QHostInfo::NoError)
    {
        auto addresses = info.addresses();
        if (addresses.count() >= 1)
        {
            setHost(addresses.first().toString());
            return;
        }
    }

    Misc::Utilities::showMessageBox(tr("IP address lookup error"), info.errorString());
}

/**
 * Displays any MQTT-related error with a GUI message-box
 */
void Client::onError(const QMQTT::ClientError error)
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
 * Registers the given @a frame data to the list of frames that shall be published
 * to the MQTT broker/server
 */
void Client::onFrameReceived(const QByteArray &frame)
{
    // Ignore if device is not connected
    if (!IO::Manager::getInstance()->connected())
        return;

    // Ignore if mode is not set to publisher
    else if (clientMode() != ClientPublisher)
        return;

    // Validate frame & append it to frame list
    if (!frame.isEmpty())
        m_frames.append(frame);
}

/**
 * Reads the given MQTT @a message and instructs the @c IO::Manager module to process
 * received data directly.
 */
void Client::onMessageReceived(const QMQTT::Message &message)
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

    // Add EOL character
    if (!mpayld.endsWith('\n'))
        mpayld.append('\n');

    // Let IO manager process incoming data
    IO::Manager::getInstance()->processPayload(mpayld);
}
}

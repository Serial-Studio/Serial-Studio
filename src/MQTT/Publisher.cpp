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

#include "Publisher.h"

#include <Misc/Utilities.h>
#include <Misc/TimerEvents.h>

using namespace MQTT;

static Client *INSTANCE = nullptr;

Client::Client()
{
    m_lookupActive = false;
    m_clientMode = MQTTClientMode::ClientPublisher;

    connect(&m_client, &QMQTT::Client::connected, this, &Client::connectedChanged);
    connect(&m_client, &QMQTT::Client::disconnected, this, &Client::connectedChanged);
    connect(&m_client, &QMQTT::Client::error, this, &Client::onError);

    setPort(defaultPort());
    setHost(defaultHost());
}

Client::~Client()
{
    disconnectFromHost();
}

Client *Client::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new Client;

    return INSTANCE;
}

quint16 Client::port() const
{
    return m_client.port();
}

QString Client::topic() const
{
    return m_topic;
}

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

int Client::clientMode() const
{
    return m_clientMode;
}

QString Client::username() const
{
    return m_client.username();
}

QString Client::password() const
{
    return QString::fromUtf8(m_client.password());
}

QString Client::host() const
{
    return m_client.host().toString();
}

bool Client::lookupActive() const
{
    return m_lookupActive;
}

bool Client::isConnectedToHost() const
{
    return m_client.isConnectedToHost();
}

QStringList Client::clientModes() const
{
    return QStringList { tr("Publisher"), tr("Suscriber") };
}

QStringList Client::mqttVersions() const
{
    return QStringList { "MQTT 3.1.0", "MQTT 3.1.1" };
}

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

void Client::setPort(const quint16 port)
{
    m_client.setPort(port);
    emit portChanged();
}

void Client::setHost(const QString &host)
{
    m_client.setHost(QHostAddress(host));
    emit hostChanged();
}

void Client::setClientMode(const int mode)
{
    m_clientMode = (MQTTClientMode)mode;
    emit clientModeChanged();
}

void Client::setTopic(const QString &topic)
{
    m_topic = topic;
    emit topicChanged();
}

void Client::setUsername(const QString &username)
{
    m_client.setUsername(username);
    emit usernameChanged();
}

void Client::setPassword(const QString &password)
{
    m_client.setPassword(password.toUtf8());
    emit passwordChanged();
}

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

void Client::sendData()
{
    // Sort JFI list from oldest to most recent
    JFI_SortList(&m_jfiList);

    // Send data in CSV format

    // Clear JFI list
    m_jfiList.clear();
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
    }

    if (!str.isEmpty())
        Misc::Utilities::showMessageBox(tr("MQTT client error"), str);
}

void Client::registerJsonFrame(const JFI_Object &frameInfo)
{
    m_jfiList.append(frameInfo);
}

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

static Publisher *INSTANCE = nullptr;

Publisher::Publisher()
{
    m_lookupActive = false;

    connect(&m_client, &QMQTT::Client::connected, this, &Publisher::connectedChanged);
    connect(&m_client, &QMQTT::Client::disconnected, this, &Publisher::connectedChanged);
    connect(&m_client, &QMQTT::Client::error, this, &Publisher::onError);

    setPort(defaultPort());
    setHost(defaultHost());
}

Publisher::~Publisher()
{
    disconnectFromHost();
}

Publisher *Publisher::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new Publisher;

    return INSTANCE;
}

quint16 Publisher::port() const
{
    return m_client.port();
}

QString Publisher::topic() const
{
    return m_topic;
}

int Publisher::mqttVersion() const
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

QString Publisher::username() const
{
    return m_client.username();
}

QString Publisher::password() const
{
    return QString::fromUtf8(m_client.password());
}

QString Publisher::host() const
{
    return m_client.host().toString();
}

bool Publisher::lookupActive() const
{
    return m_lookupActive;
}

bool Publisher::isConnectedToHost() const
{
    return m_client.isConnectedToHost();
}

QStringList Publisher::mqttVersions() const
{
    return QStringList { "MQTT 3.1.0", "MQTT 3.1.1" };
}

void Publisher::connectToHost()
{
    m_client.connectToHost();
}

/**
 * Connects/disconnects the application from the current MQTT broker. This function is
 * used as a convenience for the connect/disconnect button.
 */
void Publisher::toggleConnection()
{
    if (isConnectedToHost())
        disconnectFromHost();
    else
        connectToHost();
}

void Publisher::disconnectFromHost()
{
    m_client.disconnectFromHost();
}

/**
 * Performs a DNS lookup for the given @a host name
 */
void Publisher::lookup(const QString &host)
{
    m_lookupActive = true;
    emit lookupActiveChanged();
    QHostInfo::lookupHost(host.simplified(), this, &Publisher::lookupFinished);
}

void Publisher::setPort(const quint16 port)
{
    m_client.setPort(port);
    emit portChanged();
}

void Publisher::setHost(const QString &host)
{
    m_client.setHost(QHostAddress(host));
    emit hostChanged();
}

void Publisher::setTopic(const QString &topic)
{
    m_topic = topic;
    emit topicChanged();
}

void Publisher::setUsername(const QString &username)
{
    m_client.setUsername(username);
    emit usernameChanged();
}

void Publisher::setPassword(const QString &password)
{
    m_client.setPassword(password.toUtf8());
    emit passwordChanged();
}

void Publisher::setMqttVersion(const int versionIndex)
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

void Publisher::sendData()
{
    // Sort JFI list from oldest to most recent
    JFI_SortList(&m_jfiList);

    // Clear JFI list
}

/**
 * Sets the host IP address when the lookup finishes.
 * If the lookup fails, the error code/string shall be shown to the user in a messagebox.
 */
void Publisher::lookupFinished(const QHostInfo &info)
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

void Publisher::onError(const QMQTT::ClientError error)
{
    qDebug() << error;
}

void Publisher::registerJsonFrame(const JFI_Object &frameInfo)
{
    m_jfiList.append(frameInfo);
}

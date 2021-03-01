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

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <QObject>
#include <QHostInfo>
#include <QByteArray>
#include <QHostAddress>

#include <JSON/Frame.h>
#include <JSON/FrameInfo.h>

#include <qmqtt.h>

namespace MQTT
{
enum MQTTClientMode
{
    ClientPublisher = 0,
    ClientSubscriber = 1
};

class Client : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(quint16 port
               READ port
               WRITE setPort
               NOTIFY portChanged)
    Q_PROPERTY(QString host
               READ host
               WRITE setHost
               NOTIFY hostChanged)
    Q_PROPERTY(QString topic
               READ topic
               WRITE setTopic
               NOTIFY topicChanged)
    Q_PROPERTY(int mqttVersion
               READ mqttVersion
               WRITE setMqttVersion
               NOTIFY mqttVersionChanged)
    Q_PROPERTY(int clientMode
               READ clientMode
               WRITE setClientMode
               NOTIFY clientModeChanged)
    Q_PROPERTY(QString username
               READ username
               WRITE setUsername
               NOTIFY usernameChanged)
    Q_PROPERTY(QString password
               READ password
               WRITE setPassword
               NOTIFY passwordChanged)
    Q_PROPERTY(bool isConnectedToHost
               READ isConnectedToHost
               NOTIFY connectedChanged)
    Q_PROPERTY(bool lookupActive
               READ lookupActive
               NOTIFY lookupActiveChanged)
    Q_PROPERTY(QStringList mqttVersions
               READ mqttVersions
               CONSTANT)
    Q_PROPERTY(QStringList clientModes
               READ clientModes
               CONSTANT)
    Q_PROPERTY(quint16 defaultPort
               READ defaultPort
               CONSTANT)
    Q_PROPERTY(QString defaultHost
               READ defaultHost
               CONSTANT)
    // clang-format on

signals:
    void portChanged();
    void hostChanged();
    void topicChanged();
    void usernameChanged();
    void passwordChanged();
    void connectedChanged();
    void clientModeChanged();
    void mqttVersionChanged();
    void lookupActiveChanged();

public:
    static Client *getInstance();

    quint16 port() const;
    QString host() const;
    QString topic() const;
    int clientMode() const;
    int mqttVersion() const;
    QString username() const;
    QString password() const;
    bool lookupActive() const;
    bool isConnectedToHost() const;
    QStringList clientModes() const;
    QStringList mqttVersions() const;

    quint16 defaultPort() const { return 1883; }
    QString defaultHost() const { return "127.0.0.1"; }

public slots:
    void connectToHost();
    void toggleConnection();
    void disconnectFromHost();
    void lookup(const QString &host);
    void setPort(const quint16 port);
    void setHost(const QString &host);
    void setClientMode(const int mode);
    void setTopic(const QString &topic);
    void setUsername(const QString &username);
    void setPassword(const QString &password);
    void setMqttVersion(const int versionIndex);

private:
    Client();
    ~Client();

private slots:
    void sendData();
    void resetStatistics();
    void lookupFinished(const QHostInfo &info);
    void onError(const QMQTT::ClientError error);
    void registerJsonFrame(const JFI_Object &frameInfo);

private:
    QString m_topic;
    bool m_lookupActive;
    QMQTT::Client m_client;
    quint16 m_sentMessages;
    QList<JFI_Object> m_jfiList;
    MQTTClientMode m_clientMode;
};
}

#endif

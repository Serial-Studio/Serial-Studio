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

#pragma once

#include <QObject>
#include <QHostInfo>
#include <QByteArray>
#include <QHostAddress>

#include <qmqtt.h>

namespace MQTT
{
/**
 * @brief The MQTTClientMode enum
 *
 * Specifies the operation modes of the MQTT client
 */
enum MQTTClientMode
{
    ClientPublisher = 0,
    ClientSubscriber = 1
};

/**
 * @brief The Client class
 *
 * Implements a simple MQTT client, which allows Serial Studio to upload received frames
 * to a MQTT broker so that other devices and/or services can make use of that
 * information. By acting as a MQTT subscriber, Serial Studio can display & process frames
 * from a remote Serial Studio instance. As you might notice, this has a lot of
 * interesting applications.
 *
 * For example, you can receive frames from a CanSat mission and display them allmost in
 * real-time in another location, such as the "ground control" centre or by the media team
 * which streams the GCS display on the internet as the mission is developing.
 */
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
    Q_PROPERTY(QVector<QString> mqttVersions
               READ mqttVersions
               CONSTANT)
    Q_PROPERTY(QVector<QString> clientModes
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
    bool isSubscribed() const;
    bool isConnectedToHost() const;
    QVector<QString> clientModes() const;
    QVector<QString> mqttVersions() const;

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
    void onConnectedChanged();
    void lookupFinished(const QHostInfo &info);
    void onError(const QMQTT::ClientError error);
    void onFrameReceived(const QByteArray &frame);
    void onMessageReceived(const QMQTT::Message &message);

private:
    QString m_topic;
    bool m_lookupActive;
    QMQTT::Client m_client;
    quint16 m_sentMessages;
    QVector<QByteArray> m_frames;
    MQTTClientMode m_clientMode;
};
}

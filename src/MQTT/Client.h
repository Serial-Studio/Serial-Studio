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
#include <QSslConfiguration>

#include <qmqtt.h>
#include <DataTypes.h>

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
 * from a remote Serial Studio instance that is setup as a MQTT publisher. As you might
 * notice, this has a lot of interesting applications.
 *
 * For example, you can receive frames from a CanSat mission and display them almost in
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
    Q_PROPERTY(quint8 qos
               READ qos
               WRITE setQos
               NOTIFY qosChanged)
    Q_PROPERTY(QString host
               READ host
               WRITE setHost
               NOTIFY hostChanged)
    Q_PROPERTY(bool retain
               READ retain
               WRITE setRetain
               NOTIFY retainChanged)
    Q_PROPERTY(quint16 keepAlive
               READ keepAlive
               WRITE setKeepAlive
               NOTIFY keepAliveChanged)
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
    Q_PROPERTY(bool sslEnabled
               READ sslEnabled
               WRITE setSslEnabled
               NOTIFY sslEnabledChanged)
    Q_PROPERTY(int sslProtocol
               READ sslProtocol
               WRITE setSslProtocol
               NOTIFY sslProtocolChanged)
    Q_PROPERTY(bool isConnectedToHost
               READ isConnectedToHost
               NOTIFY connectedChanged)
    Q_PROPERTY(bool lookupActive
               READ lookupActive
               NOTIFY lookupActiveChanged)
    Q_PROPERTY(StringList mqttVersions
               READ mqttVersions
               CONSTANT)
    Q_PROPERTY(StringList clientModes
               READ clientModes
               CONSTANT)
    Q_PROPERTY(StringList qosLevels
               READ qosLevels
               CONSTANT)
    Q_PROPERTY(StringList sslProtocols
               READ sslProtocols
               CONSTANT)
    Q_PROPERTY(quint16 defaultPort
               READ defaultPort
               CONSTANT)
    Q_PROPERTY(QString defaultHost
               READ defaultHost
               CONSTANT)
    Q_PROPERTY(QString caFilePath
               READ caFilePath
               NOTIFY caFilePathChanged)
    // clang-format on

Q_SIGNALS:
    void qosChanged();
    void portChanged();
    void hostChanged();
    void topicChanged();
    void retainChanged();
    void usernameChanged();
    void passwordChanged();
    void keepAliveChanged();
    void connectedChanged();
    void caFilePathChanged();
    void clientModeChanged();
    void sslEnabledChanged();
    void sslProtocolChanged();
    void mqttVersionChanged();
    void lookupActiveChanged();

private:
    explicit Client();
    Client(Client &&) = delete;
    Client(const Client &) = delete;
    Client &operator=(Client &&) = delete;
    Client &operator=(const Client &) = delete;

    ~Client();

public:
    static Client &instance();

    quint8 qos() const;
    bool retain() const;
    quint16 port() const;
    QString host() const;
    QString topic() const;
    int clientMode() const;
    int sslProtocol() const;
    int mqttVersion() const;
    bool sslEnabled() const;
    QString username() const;
    QString password() const;
    quint16 keepAlive() const;
    bool lookupActive() const;
    bool isSubscribed() const;
    bool isConnectedToHost() const;

    StringList qosLevels() const;
    StringList clientModes() const;
    StringList mqttVersions() const;
    StringList sslProtocols() const;

    QString caFilePath() const;
    quint16 defaultPort() const { return 1883; }
    QString defaultHost() const { return "127.0.0.1"; }

public Q_SLOTS:
    void loadCaFile();
    void connectToHost();
    void toggleConnection();
    void disconnectFromHost();
    void setQos(const quint8 qos);
    void lookup(const QString &host);
    void setPort(const quint16 port);
    void setHost(const QString &host);
    void setRetain(const bool retain);
    void setClientMode(const int mode);
    void setTopic(const QString &topic);
    void loadCaFile(const QString &path);
    void setSslProtocol(const int index);
    void setSslEnabled(const bool enabled);
    void setUsername(const QString &username);
    void setPassword(const QString &password);
    void setKeepAlive(const quint16 keepAlive);
    void setMqttVersion(const int versionIndex);

private Q_SLOTS:
    void sendData();
    void resetStatistics();
    void onConnectedChanged();
    void lookupFinished(const QHostInfo &info);
    void onError(const QMQTT::ClientError error);
    void onFrameReceived(const QByteArray &frame);
    void onSslErrors(const QList<QSslError> &errors);
    void onMessageReceived(const QMQTT::Message &message);

private:
    void regenerateClient();

private:
    QString m_topic;
    bool m_sslEnabled;
    int m_sslProtocol;
    bool m_lookupActive;
    QString m_caFilePath;
    quint16 m_sentMessages;
    MQTTClientMode m_clientMode;
    QVector<QByteArray> m_frames;
    QPointer<QMQTT::Client> m_client;
    QSslConfiguration m_sslConfiguration;
};
}

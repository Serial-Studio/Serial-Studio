QMQTT
=====

mqtt client for Qt 5 in maintenance status

**Please compile the library with Qt >= 5.3 version. On Windows you need to specify `CONFIG += NO_UNIT_TESTS`, since gtest is not supported.**

SSL is enabled by default, if the version of OpenSSL < 1.0.2, SSL may not be supported.

Disable the SSL in CMakeLists.txt (cmake):

    option( ${PROJECT_NAME}_SSL "Enable SSL support for MQTT" OFF )

Disable the SSL with src/mqtt/qmqtt.pro (qmake):

    CONFIG += QMQTT_NO_SSL

To add websocket support, compile the library with Qt >= 5.7, and specify 'CONFIG += QMQTT_WEBSOCKETS'.
This also works when compiling qmqtt for WebAssembly.

Usage
=====

In your QMake project, add:

    QT += qmqtt

Connect using TCP:

    #include "qmqtt.h"

    QMQTT::Client *client = new QMQTT::Client(QHostAddress::LocalHost, 1883);
    client->setClientId("clientId");
    client->setUsername("user");
    client->setPassword("password");
    client->connectToHost();

Connect using SSL:

    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    // Add custom SSL options here (for example extra certificates)
    QMQTT::Client *client = new QMQTT::Client("example.com", 8883, sslConfig);
    client->setClientId("clientId");
    client->setUsername("user");
    client->setPassword("password");
    // Optionally, set ssl errors you want to ignore. Be careful, because this may weaken security.
    // See QSslSocket::ignoreSslErrors(const QList<QSslError> &) for more information.
    client->ignoreSslErrors(<list of expected ssl errors>)
    client->connectToHost();
    // Here's another option to suppress SSL errors (again, be careful)
    QObject::connect(client, &QMQTT::Client::sslErrors, [&](const QList<QSslError> &errors) {
        // Investigate the errors here, if you find no serious problems, call ignoreSslErrors()
        // to continue connecting.
        client->ignoreSslErrors();
    });

Connect using WebSockets:

    QMQTT::Client *client = new QMQTT::Client("ws://www.example.com/mqtt", "<origin>", QWebSocketProtocol::VersionLatest);
    client->setClientId("clientId");
    client->setUsername("user");
    client->setPassword("password");
    client->connectToHost();

Slots
=====

    void setHost(const QHostAddress& host);
    void setPort(const quint16 port);
    void setClientId(const QString& clientId);
    void setUsername(const QString& username);
    void setPassword(const QString& password);
    void setKeepAlive(const int keepAlive);
    void setCleanSession(const bool cleansess);
    void setAutoReconnect(const bool value);
    void setAutoReconnectInterval(const int autoReconnectInterval);
    void setWillTopic(const QString& willTopic);
    void setWillQos(const quint8 willQos);
    void setWillRetain(const bool willRetain);
    void setWillMessage(const QString& willMessage);

    void connectToHost();
    void disconnectFromHost();

    quint16 subscribe(const QString& topic, const quint8 qos);
    void unsubscribe(const QString& topic);

    quint16 publish(const Message& message);

Signals
=======

    void connected();
    void disconnected();
    void error(const QMQTT::ClientError error);

    void subscribed(const QString& topic, const quint8 qos);
    void unsubscribed(const QString& topic);
    void published(const quint16 msgid, const quint8 qos);
    void pingresp();
    void received(const QMQTT::Message& message);

Qt 6 support
============

This library has been developed and tested against Qt 5. Active work on it has stopped since Qt released its own _mqtt_ module several years ago. There are currently no plans for further extensions except for smaller rectifications and bug fixing. At your own risk you may use it in Qt6 projects using cmake.


License
=======

New BSD License

Contributors
=============

[@avsdev-cw](https://github.com/avsdev-cw),
[@alex-spataru](https://github.com/alex-spataru),
Benjamin Schmitz,
[@bf-bryants](https://github.com/bf-bryants),
[@bog-dan-ro](https://github.com/bog-dan-ro),
[@chsterz](https://github.com/chsterz),
[@cncap](https://github.com/cncap),
[@ejvr](https://github.com/ejvr),
[@ehntoo](https://github.com/ehntoo),
Erik Botö,
Guillaume Bour,
[@halfgaar](https://github.com/halfgaar),
[@hxcan](https://github.com/hxcan),
[@jpnurmi](https://github.com/jpnurmi),
Kai Dohmen,
[@Kampfgnom](https://github.com/Kampfgnom),
[@keytee](https://github.com/keytee),
[@kollix](https://github.com/kollix),
[@KonstantinRitt](https://github.com/KonstantinRitt),
[@maggu2810](https://github.com/maggu2810),
[@mwallnoefer](https://github.com/mwallnoefer),
[@NicoWallmeier](https://github.com/NicoWallmeier),
Niklas Wulf,
Niraj Desai,
[@Psy-Kai](https://github.com/Psy-Kai),
[@rafaeldelucena](https://github.com/rafaeldelucena),
[@rokm](https://github.com/rokm),
[@RoachLin](https://github.com/RoachLin),
[@sergey-kuzminov](https://github.com/sergey-kuzminov),
sgaoemq,
[@Vortex375](https://github.com/Vortex375),
ybq

Authors
=======

[@emqplus](https://github.com/emqplus) Feng Lee <feng@emqx.io>

[@wguynes](https://github.com/wguynes) William Guynes <wguynes@gmail.com>

[@wuming123057](https://github.com/wuming123057) Xuan <huacai123057@163.com>

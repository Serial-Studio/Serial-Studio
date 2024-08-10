#ifdef QT_WEBSOCKETS_LIB

#  include "qmqtt_websocket_p.h"

#  include <QNetworkRequest>
#  include <QUrl>
#  include <QSslError>

#  ifndef QT_NO_SSL
QMQTT::WebSocket::WebSocket(const QString &origin,
                            QWebSocketProtocol::Version version,
                            const QSslConfiguration *sslConfig, QObject *parent)
  : SocketInterface(parent)
  , _socket(new QWebSocket(origin, version, this))
  , _ioDevice(new WebSocketIODevice(_socket, this))
{
  initialize();
  if (sslConfig != nullptr)
    _socket->setSslConfiguration(*sslConfig);
  connect(_socket, &QWebSocket::sslErrors, this, &WebSocket::sslErrors);
}
#  endif // QT_NO_SSL

QMQTT::WebSocket::WebSocket(const QString &origin,
                            QWebSocketProtocol::Version version,
                            QObject *parent)
  : SocketInterface(parent)
  , _socket(new QWebSocket(origin, version, this))
  , _ioDevice(new WebSocketIODevice(_socket, this))
{
  initialize();
}

void QMQTT::WebSocket::initialize()
{
  connect(_socket, &QWebSocket::connected, this, &WebSocket::connected);
  connect(_socket, &QWebSocket::disconnected, this, &WebSocket::disconnected);
  connect(_socket,
          static_cast<void (QWebSocket::*)(QAbstractSocket::SocketError)>(
              &QWebSocket::error),
          this,
          static_cast<void (SocketInterface::*)(QAbstractSocket::SocketError)>(
              &SocketInterface::error));
}

QMQTT::WebSocket::~WebSocket() {}

void QMQTT::WebSocket::connectToHost(const QHostAddress &address, quint16 port)
{
  Q_UNUSED(address)
  Q_UNUSED(port)
  qFatal("No supported");
}

void QMQTT::WebSocket::connectToHost(const QString &hostName, quint16 port)
{
  Q_UNUSED(port)
  QUrl url(hostName);
  QNetworkRequest request(url);
  request.setRawHeader("Sec-WebSocket-Protocol", "mqtt");
  _ioDevice->connectToHost(request);
}

void QMQTT::WebSocket::disconnectFromHost()
{
  _socket->close();
}

QAbstractSocket::SocketState QMQTT::WebSocket::state() const
{
  return _socket->state();
}

QAbstractSocket::SocketError QMQTT::WebSocket::error() const
{
  return _socket->error();
}

#  ifndef QT_NO_SSL
void QMQTT::WebSocket::ignoreSslErrors(const QList<QSslError> &errors)
{
  _socket->ignoreSslErrors(errors);
}

void QMQTT::WebSocket::ignoreSslErrors()
{
  _socket->ignoreSslErrors();
}

QSslConfiguration QMQTT::WebSocket::sslConfiguration() const
{
  return _socket->sslConfiguration();
}

void QMQTT::WebSocket::setSslConfiguration(const QSslConfiguration &config)
{
  _socket->setSslConfiguration(config);
}

#  endif // QT_NO_SSL

#endif // QT_WEBSOCKETS_LIB

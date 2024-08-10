#ifndef NETWORK_MOCK_H
#define NETWORK_MOCK_H

#include <qmqtt_networkinterface.h>
#include <gmock/gmock.h>
#ifndef QT_NO_SSL
#  include <QSslSocket>
#endif // QT_NO_SSL

class NetworkMock : public QMQTT::NetworkInterface
{
public:
  MOCK_METHOD1(sendFrame, void(const QMQTT::Frame &));
  MOCK_CONST_METHOD0(isConnectedToHost, bool());
  MOCK_CONST_METHOD0(autoReconnect, bool());
  MOCK_METHOD1(setAutoReconnect, void(const bool));
  MOCK_CONST_METHOD0(autoReconnectInterval, int());
  MOCK_METHOD1(setAutoReconnectInterval, void(const int));
  MOCK_CONST_METHOD0(state, QAbstractSocket::SocketState());
  MOCK_METHOD2(connectToHost, void(const QHostAddress &, const quint16));
  MOCK_METHOD2(connectToHost, void(const QString &, const quint16));
  MOCK_METHOD0(disconnectFromHost, void());
#ifndef QT_NO_SSL
  MOCK_METHOD1(ignoreSslErrors, void(const QList<QSslError> &));
  MOCK_METHOD0(ignoreSslErrors, void());
  MOCK_CONST_METHOD0(sslConfiguration, QSslConfiguration());
  MOCK_METHOD1(setSslConfiguration, void(const QSslConfiguration &));
#endif // QT_NO_SSL
};

#endif // NETWORK_MOCK_H

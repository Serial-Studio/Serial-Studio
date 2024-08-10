#ifndef SOCKET_MOCK_H
#define SOCKET_MOCK_H

#include <qmqtt_socketinterface.h>
#include <gmock/gmock.h>
#include <QScopedPointer>
#include "iodevicemock.h"

class SocketMock : public QMQTT::SocketInterface
{
public:
  SocketMock(QObject *parent = nullptr)
    : QMQTT::SocketInterface(parent)
    , mockIoDevice(new IODeviceMock)
  {
    mockIoDevice->open(QIODevice::ReadWrite);
  }

  QScopedPointer<IODeviceMock> mockIoDevice;

  virtual QIODevice *ioDevice() { return mockIoDevice.data(); }

  using QMQTT::SocketInterface::error;
  MOCK_METHOD2(connectToHost, void(const QHostAddress &, quint16));
  MOCK_METHOD2(connectToHost, void(const QString &, quint16));
  MOCK_METHOD0(disconnectFromHost, void());
  MOCK_CONST_METHOD0(state, QAbstractSocket::SocketState());
  MOCK_CONST_METHOD0(error, QAbstractSocket::SocketError());
};

#endif // SOCKET_MOCK_H

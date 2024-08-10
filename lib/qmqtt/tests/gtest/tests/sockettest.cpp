#include <qmqtt_socket_p.h>
#include "tcpserver.h"
#include <QCoreApplication>
#include <QSignalSpy>
#include <QSharedPointer>
#include <QDataStream>
#include <QTcpSocket>
#include <gmock/gmock.h>

using namespace testing;

const QHostAddress HOST = QHostAddress::LocalHost;
const quint16 PORT = 3875;
const QByteArray BYTE_ARRAY
    = QByteArrayLiteral("Supercalifragilisticexpialidocious");
const int TCP_TIMEOUT_MS = 5000;

class SocketTest : public Test
{
public:
  explicit SocketTest()
    : _socket(new QMQTT::Socket)
  {
  }
  virtual ~SocketTest() {}

  QScopedPointer<QMQTT::Socket> _socket;

  void flushEvents()
  {
    while (QCoreApplication::hasPendingEvents())
    {
      QCoreApplication::processEvents(QEventLoop::AllEvents);
      QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    }
  }

  QSharedPointer<TcpServer> createAndConnectToServer()
  {
    QSharedPointer<TcpServer> server
        = QSharedPointer<TcpServer>(new TcpServer(HOST, PORT));
    _socket->connectToHost(server->serverAddress(), server->serverPort());
    bool timedOut = false;
    bool connectionMade
        = server->waitForNewConnection(TCP_TIMEOUT_MS, &timedOut);
    EXPECT_TRUE(connectionMade);
    EXPECT_FALSE(timedOut);
    EXPECT_THAT(server->socket(), NotNull());
    flushEvents();
    EXPECT_EQ(QAbstractSocket::ConnectedState, server->socket()->state());
    EXPECT_EQ(QAbstractSocket::ConnectedState, _socket->state());
    return server;
  }
};

TEST_F(SocketTest, defaultConstructor_Test)
{
  EXPECT_FALSE(_socket.isNull());
  EXPECT_EQ(QAbstractSocket::UnconnectedState, _socket->state());
}

TEST_F(SocketTest, connectToHostMakesTcpConnection_Test)
{
  TcpServer server(HOST, PORT);
  _socket->connectToHost(server.serverAddress(), server.serverPort());
  bool timedOut = false;
  bool connectionMade = server.waitForNewConnection(TCP_TIMEOUT_MS, &timedOut);

  EXPECT_TRUE(connectionMade);
  EXPECT_FALSE(timedOut);
}

TEST_F(SocketTest, disconnectFromHostDisconnectsTcpConnection_Test)
{
  QSharedPointer<TcpServer> server = createAndConnectToServer();
  QSignalSpy spy(server->socket(), &QTcpSocket::disconnected);

  _socket->disconnectFromHost();
  flushEvents();

  EXPECT_EQ(1, spy.count());
}

TEST_F(SocketTest, tcpConnectionEmitsConnectedSignal_Test)
{
  QSignalSpy spy(_socket.data(), &QMQTT::Socket::connected);

  createAndConnectToServer();
  flushEvents();

  EXPECT_EQ(1, spy.count());
}

TEST_F(SocketTest, tcpDisconnectionEmitsDisconnectedSignal_Test)
{
  createAndConnectToServer();
  QSignalSpy spy(_socket.data(), &QMQTT::Socket::disconnected);

  _socket->disconnectFromHost();
  flushEvents();

  EXPECT_EQ(1, spy.count());
}

TEST_F(SocketTest, remoteDisconnectionEmitsDisconnectedSignal_Test)
{
  QSharedPointer<TcpServer> server = createAndConnectToServer();
  QSignalSpy spy(_socket.data(), &QMQTT::Socket::disconnected);

  server->socket()->disconnectFromHost();
  flushEvents();

  EXPECT_EQ(1, spy.count());
}

TEST_F(SocketTest, connectedSocketShowsConnectedState_Test)
{
  QSharedPointer<TcpServer> server = createAndConnectToServer();
  flushEvents();
  EXPECT_EQ(QAbstractSocket::ConnectedState, _socket->state());
}

TEST_F(SocketTest, incomingDataTriggersReadyReadSignal_Test)
{
  QSharedPointer<TcpServer> server = createAndConnectToServer();
  QSignalSpy spy(_socket->ioDevice(), &QIODevice::readyRead);

  server->socket()->write(BYTE_ARRAY);
  flushEvents();

  EXPECT_EQ(1, spy.count());
}

TEST_F(SocketTest, pendingDataOnSocketReturnsAtEndFalse_Test)
{
  QSharedPointer<TcpServer> server = createAndConnectToServer();
  server->socket()->write(BYTE_ARRAY);
  server->socket()->waitForBytesWritten(TCP_TIMEOUT_MS);
  flushEvents();

  EXPECT_FALSE(_socket->ioDevice()->atEnd());
}

TEST_F(SocketTest, noPendingDataOnSocketReturnsAtEndTrue_Test)
{
  QSharedPointer<TcpServer> server = createAndConnectToServer();
  flushEvents();

  EXPECT_TRUE(_socket->ioDevice()->atEnd());
}

TEST_F(SocketTest, incomingDataIsRetrivable_Test)
{
  QSharedPointer<TcpServer> server = createAndConnectToServer();
  QSignalSpy spy(_socket->ioDevice(), &QIODevice::readyRead);

  server->socket()->write(BYTE_ARRAY);
  flushEvents();

  EXPECT_EQ(1, spy.count());
  EXPECT_EQ(BYTE_ARRAY, _socket->ioDevice()->read(Q_INT64_C(1000000)));
}

TEST_F(SocketTest, socketErrorEmitsErrorSignal_Test)
{
  QSignalSpy spy(_socket.data(), static_cast<void (QMQTT::SocketInterface::*)(
                                     QAbstractSocket::SocketError)>(
                                     &QMQTT::SocketInterface::error));
  _socket->connectToHost(HOST, PORT);
  flushEvents();

  EXPECT_EQ(1, spy.count());
  EXPECT_EQ(QAbstractSocket::ConnectionRefusedError,
            spy.at(0).at(0).value<QAbstractSocket::SocketError>());
}

TEST_F(SocketTest, errorGivesSocketErrorReason_Test)
{
  _socket->connectToHost(HOST, PORT);
  flushEvents();

  EXPECT_EQ(QAbstractSocket::ConnectionRefusedError, _socket->error());
}

TEST_F(SocketTest, socketSendsOutgoingDataUsingQDataStream_Test)
{
  QTcpServer server;
  server.listen(HOST, PORT);
  _socket->connectToHost(HOST, PORT);
  bool timedOut = false;
  EXPECT_TRUE(server.waitForNewConnection(TCP_TIMEOUT_MS, &timedOut));
  EXPECT_FALSE(timedOut);
  QTcpSocket *serverSocket = server.nextPendingConnection();
  EXPECT_THAT(serverSocket, NotNull());
  flushEvents();
  EXPECT_EQ(QAbstractSocket::ConnectedState, _socket->state());
  EXPECT_EQ(QAbstractSocket::ConnectedState, serverSocket->state());

  QDataStream out(_socket->ioDevice());
  out << static_cast<quint32>(42);
  EXPECT_TRUE(_socket->ioDevice()->waitForBytesWritten(TCP_TIMEOUT_MS));
  EXPECT_TRUE(serverSocket->waitForReadyRead(TCP_TIMEOUT_MS));

  quint32 i = 0;
  QDataStream in(serverSocket);
  in >> i;
  EXPECT_EQ(static_cast<quint32>(42), i);
}

TEST_F(SocketTest, socketReceivesIncomingDataUsingQDataStream_Test)
{
  QTcpServer server;
  server.listen(HOST, PORT);
  _socket->connectToHost(HOST, PORT);
  bool timedOut = false;
  EXPECT_TRUE(server.waitForNewConnection(TCP_TIMEOUT_MS, &timedOut));
  EXPECT_FALSE(timedOut);
  QTcpSocket *serverSocket = server.nextPendingConnection();
  EXPECT_THAT(serverSocket, NotNull());
  flushEvents();
  EXPECT_EQ(QAbstractSocket::ConnectedState, _socket->state());
  EXPECT_EQ(QAbstractSocket::ConnectedState, serverSocket->state());

  QDataStream out(serverSocket);
  out << static_cast<quint32>(42);
  EXPECT_TRUE(serverSocket->waitForBytesWritten(TCP_TIMEOUT_MS));
  EXPECT_TRUE(_socket->ioDevice()->waitForReadyRead(TCP_TIMEOUT_MS));

  quint32 i = 0;
  QDataStream in(_socket->ioDevice());
  in >> i;
  EXPECT_EQ(static_cast<quint32>(42), i);
}

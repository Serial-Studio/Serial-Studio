#include "tcpserver.h"
#include <QHostAddress>
#include <QTcpSocket>

TcpServer::TcpServer(const QHostAddress host, const quint16 port)
  : _socket(nullptr)
{
  connect(this, &QTcpServer::newConnection, this, &TcpServer::on_newConnection);
  listen(host, port);
}

TcpServer::~TcpServer() {}

void TcpServer::on_newConnection()
{
  if (nullptr != _socket)
  {
    disconnect(_socket, &QTcpSocket::readyRead, this, &TcpServer::on_readyRead);
    _socket->disconnectFromHost();
    _socket->deleteLater();
    _socket = nullptr;
    _data.clear();
  }
  _socket = nextPendingConnection();
  if (nullptr != _socket)
  {
    connect(_socket, &QTcpSocket::readyRead, this, &TcpServer::on_readyRead);
  }
}

void TcpServer::on_readyRead()
{
  _data.append(_socket->readAll());
}

QByteArray TcpServer::data() const
{
  return _data;
}

QTcpSocket *TcpServer::socket() const
{
  return _socket;
}

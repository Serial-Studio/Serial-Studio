#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <QTcpServer>
#include <QHostAddress>

class TcpServer : public QTcpServer
{
  Q_OBJECT
public:
  explicit TcpServer(const QHostAddress host, const quint16 port);
  virtual ~TcpServer();

  QByteArray data() const;
  QTcpSocket *socket() const;

  static const QHostAddress HOST;
  static const quint16 PORT;

protected:
  QTcpSocket *_socket;
  QByteArray _data;

protected Q_SLOTS:
  void on_newConnection();
  void on_readyRead();
};

#endif // TCP_SERVER_H

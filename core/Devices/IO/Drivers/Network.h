/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QAbstractSocket>
#include <QByteArray>
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QSettings>
#include <QSslError>
#include <QTcpSocket>
#include <QTimer>
#include <QUdpSocket>
#include <QUrl>
#include <QWebSocket>

#include "IO/AsyncTcpDial.h"
#include "IO/HAL_Driver.h"

namespace IO {
namespace Drivers {
/**
 * @brief HAL driver for network sources: TCP and UDP sockets, a WebSocket client, and an HTTP
 *        client that polls a REST endpoint. All four are clients; nothing here listens. Each
 *        transport implements itself in its own translation unit under Drivers/Network/, and
 *        this class is the facade plus the dispatch between them.
 */
class Network : public HAL_Driver {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString remoteAddress
             READ remoteAddress
             WRITE setRemoteAddress
             NOTIFY addressChanged)
  Q_PROPERTY(quint16 tcpPort
             READ tcpPort
             WRITE setTcpPort
             NOTIFY portChanged)
  Q_PROPERTY(quint16 udpLocalPort
             READ udpLocalPort
             WRITE setUdpLocalPort
             NOTIFY portChanged)
  Q_PROPERTY(quint16 udpRemotePort
             READ udpRemotePort
             WRITE setUdpRemotePort
             NOTIFY portChanged)
  Q_PROPERTY(IO::Drivers::Network::SocketType socketType
             READ socketType
             WRITE setSocketType
             NOTIFY socketTypeChanged)
  Q_PROPERTY(int socketTypeIndex
             READ socketTypeIndex
             WRITE setSocketTypeIndex
             NOTIFY socketTypeChanged)
  Q_PROPERTY(QStringList socketTypes
             READ socketTypes
             CONSTANT)
  Q_PROPERTY(QString defaultAddress
             READ defaultAddress
             CONSTANT)
  Q_PROPERTY(quint16 defaultTcpPort
             READ defaultTcpPort
             CONSTANT)
  Q_PROPERTY(quint16 defaultUdpLocalPort
             READ defaultUdpLocalPort
             CONSTANT)
  Q_PROPERTY(quint16 defaultUdpRemotePort
             READ defaultUdpRemotePort
             CONSTANT)
  Q_PROPERTY(bool lookupActive
             READ lookupActive
             NOTIFY lookupActiveChanged)
  Q_PROPERTY(bool udpMulticast
             READ udpMulticast
             WRITE setUdpMulticast
             NOTIFY udpMulticastChanged)
  Q_PROPERTY(QString webSocketUrl
             READ webSocketUrl
             WRITE setWebSocketUrl
             NOTIFY webSocketChanged)
  Q_PROPERTY(int webSocketFormatIndex
             READ webSocketFormatIndex
             WRITE setWebSocketFormatIndex
             NOTIFY webSocketChanged)
  Q_PROPERTY(QStringList webSocketFormats
             READ webSocketFormats
             CONSTANT)
  Q_PROPERTY(QString defaultWebSocketUrl
             READ defaultWebSocketUrl
             CONSTANT)
  Q_PROPERTY(bool ignoreTlsErrors
             READ ignoreTlsErrors
             WRITE setIgnoreTlsErrors
             NOTIFY ignoreTlsErrorsChanged)
  Q_PROPERTY(QString httpUrl
             READ httpUrl
             WRITE setHttpUrl
             NOTIFY httpChanged)
  Q_PROPERTY(int httpMethodIndex
             READ httpMethodIndex
             WRITE setHttpMethodIndex
             NOTIFY httpChanged)
  Q_PROPERTY(QStringList httpMethods
             READ httpMethods
             CONSTANT)
  Q_PROPERTY(QString httpBody
             READ httpBody
             WRITE setHttpBody
             NOTIFY httpChanged)
  Q_PROPERTY(QString httpHeaders
             READ httpHeaders
             WRITE setHttpHeaders
             NOTIFY httpChanged)
  Q_PROPERTY(int httpInterval
             READ httpInterval
             WRITE setHttpInterval
             NOTIFY httpChanged)
  Q_PROPERTY(QString defaultHttpUrl
             READ defaultHttpUrl
             CONSTANT)
  Q_PROPERTY(int defaultHttpInterval
             READ defaultHttpInterval
             CONSTANT)
  // clang-format on

signals:
  void portChanged();
  void httpChanged();
  void addressChanged();
  void webSocketChanged();
  void socketTypeChanged();
  void udpMulticastChanged();
  void lookupActiveChanged();
  void ignoreTlsErrorsChanged();

public:
  explicit Network();
  ~Network() override;

  Network(Network&&)                 = delete;
  Network(const Network&)            = delete;
  Network& operator=(Network&&)      = delete;
  Network& operator=(const Network&) = delete;

  /**
   * @brief Transport the driver speaks. Qt's socket-type enum cannot name a WebSocket or an HTTP
   *        client, so the driver carries its own. The numbering IS the socket type index that
   *        project files, the JSON-RPC API and the CLI persist: append only, never renumber.
   */
  enum SocketType {
    Tcp       = 0,
    Udp       = 1,
    WebSocket = 2,
    Http      = 3
  };
  Q_ENUM(SocketType)

  void close() override;

  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isConnecting() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;
  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override;

  [[nodiscard]] quint16 tcpPort() const;
  [[nodiscard]] quint16 udpLocalPort() const;
  [[nodiscard]] quint16 udpRemotePort() const;

  [[nodiscard]] bool udpMulticast() const;
  [[nodiscard]] bool lookupActive() const;
  [[nodiscard]] int socketTypeIndex() const;
  [[nodiscard]] bool ignoreTlsErrors() const;
  [[nodiscard]] SocketType socketType() const;
  [[nodiscard]] int httpInterval() const;
  [[nodiscard]] quint64 pollsOk() const;
  [[nodiscard]] int httpMethodIndex() const;
  [[nodiscard]] quint64 pollsFailed() const;
  [[nodiscard]] quint64 pollsSkipped() const;
  [[nodiscard]] const QString& httpUrl() const;
  [[nodiscard]] const QString& httpBody() const;
  [[nodiscard]] int webSocketFormatIndex() const;
  [[nodiscard]] quint64 consecutiveFailures() const;
  [[nodiscard]] const QString& httpHeaders() const;
  [[nodiscard]] const QString& webSocketUrl() const;

  [[nodiscard]] QTcpSocket* tcpSocket() { return m_tcpSocket; }

  [[nodiscard]] QUdpSocket* udpSocket() { return m_udpSocket; }

  [[nodiscard]] const QString& remoteAddress() const;
  [[nodiscard]] QStringList httpMethods() const;
  [[nodiscard]] QStringList socketTypes() const;
  [[nodiscard]] QStringList webSocketFormats() const;

  static quint16 defaultTcpPort() { return 23; }

  static quint16 defaultUdpLocalPort() { return 0; }

  static quint16 defaultUdpRemotePort() { return 53; }

  static const QString& defaultAddress()
  {
    static QString addr = QStringLiteral("127.0.0.1");
    return addr;
  }

  static const QString& defaultWebSocketUrl()
  {
    static QString url = QStringLiteral("ws://127.0.0.1:8080");
    return url;
  }

  static int defaultHttpInterval() { return 1000; }

  static const QString& defaultHttpUrl()
  {
    static QString url = QStringLiteral("http://127.0.0.1:8080/");
    return url;
  }

public slots:
  void setDriverProperty(const QString& key, const QVariant& value) override;
  void setTcpSocket();
  void setUdpSocket();
  void lookup(const QString& host);
  void setTcpPort(const quint16 port);
  void setUdpLocalPort(const quint16 port);
  void setUdpMulticast(const bool enabled);
  void setSocketTypeIndex(const int index);
  void setUdpRemotePort(const quint16 port);
  void setRemoteAddress(const QString& address);
  void setSocketType(const SocketType type);
  void setIgnoreTlsErrors(const bool enabled);
  void setHttpUrl(const QString& url);
  void setHttpBody(const QString& body);
  void setHttpInterval(const int interval);
  void setHttpHeaders(const QString& headers);
  void setHttpMethodIndex(const int index);
  void setWebSocketUrl(const QString& url);
  void setWebSocketFormatIndex(const int index);

private slots:
  void onTcpError();
  void onUdpReadyRead();
  void onTcpReadyRead();
  void onTcpStateChanged();
  void onTcpDialFinished(bool ok, const QString& reason);
  void onPollTimeout();
  void onHttpReplyFinished();
  void onWebSocketConnected();
  void onWebSocketDisconnected();
  void lookupFinished(const QHostInfo& info);
  void onWebSocketTextMessage(const QString& message);
  void onWebSocketBinaryMessage(const QByteArray& message);
  void onWebSocketSslErrors(const QList<QSslError>& errors);
  void onHttpSslErrors(QNetworkReply* reply, const QList<QSslError>& errors);
  void onUdpError(const QAbstractSocket::SocketError socketError);
  void onWebSocketError(const QAbstractSocket::SocketError socketError);

private:
  void closeTcp();
  void closeUdp();
  void closeHttp();
  void succeedDial();
  void closeWebSocket();
  void sendHttpRequest(const QByteArray& body);
  void enlargeUdpReceiveBuffer();
  void failDial(const QString& reason);
  void reportLinkError(const QString& error);
  void appendTcpProperties(QList<IO::DriverProperty>& props) const;
  void appendUdpProperties(QList<IO::DriverProperty>& props) const;
  void appendTlsProperty(QList<IO::DriverProperty>& props) const;
  void appendAddressProperty(QList<IO::DriverProperty>& props) const;
  void appendSocketTypeProperty(QList<IO::DriverProperty>& props) const;
  void appendHttpProperties(QList<IO::DriverProperty>& props) const;
  void appendWebSocketProperties(QList<IO::DriverProperty>& props) const;

  [[nodiscard]] bool tcpOpen() const;
  [[nodiscard]] bool udpOpen() const;
  [[nodiscard]] bool tcpLinkUp() const;
  [[nodiscard]] bool tcpReadable() const;
  [[nodiscard]] bool tcpWritable() const;
  [[nodiscard]] bool udpReadable() const;
  [[nodiscard]] bool udpWritable() const;
  [[nodiscard]] bool tcpConfigured() const;
  [[nodiscard]] bool udpConfigured() const;
  [[nodiscard]] qint64 writeTcp(const QByteArray& data);
  [[nodiscard]] qint64 queueTcpWrite(const QByteArray& data);
  [[nodiscard]] qint64 writeUdp(const QByteArray& data);
  [[nodiscard]] bool openTcp(const QIODevice::OpenMode mode);
  [[nodiscard]] bool openUdp(const QIODevice::OpenMode mode);
  [[nodiscard]] bool httpOpen() const;
  [[nodiscard]] bool httpConfigured() const;
  [[nodiscard]] QByteArray readCappedBody(QNetworkReply* reply);
  [[nodiscard]] qint64 writeHttp(const QByteArray& data);
  [[nodiscard]] bool openHttp(const QIODevice::OpenMode mode);
  [[nodiscard]] QNetworkRequest buildHttpRequest(const QUrl& url) const;
  [[nodiscard]] bool applyHttpProperty(const QString& key, const QVariant& value);
  [[nodiscard]] bool webSocketOpen() const;
  [[nodiscard]] bool webSocketWritable() const;
  [[nodiscard]] bool webSocketConfigured() const;
  [[nodiscard]] qint64 writeWebSocket(const QByteArray& data);
  [[nodiscard]] bool openWebSocket(const QIODevice::OpenMode mode);
  [[nodiscard]] bool applyTcpProperty(const QString& key, const QVariant& value);
  [[nodiscard]] bool applyUdpProperty(const QString& key, const QVariant& value);
  [[nodiscard]] bool applyTlsProperty(const QString& key, const QVariant& value);
  [[nodiscard]] bool applyAddressProperty(const QString& key, const QVariant& value);
  [[nodiscard]] bool urlForCurrentMode(QUrl& url, QString& reason) const;
  [[nodiscard]] bool applyWebSocketProperty(const QString& key, const QVariant& value);
  [[nodiscard]] bool dialTcpAsync(const QString& host, const QIODevice::OpenMode mode);
  [[nodiscard]] static QHostAddress preferredAddress(const QList<QHostAddress>& addresses);

private:
  QSettings m_settings;

  QString m_address;
  QString m_pendingLookup;
  QHostAddress m_resolvedAddress;
  quint16 m_tcpPort;
  bool m_udpMulticast;
  bool m_lookupActive;
  int m_lookupId;
  quint16 m_udpLocalPort;
  quint16 m_udpRemotePort;
  SocketType m_socketType;

  QString m_webSocketUrl;
  int m_webSocketFormat;
  bool m_ignoreTlsErrors;
  bool m_dialPending;

  QString m_httpUrl;
  QString m_httpBody;
  QString m_httpHeaders;
  int m_httpMethod;
  int m_httpInterval;
  bool m_httpActive;
  bool m_httpFailureLogged;
  bool m_httpTruncationLogged;
  quint64 m_pollsOk;
  quint64 m_pollsFailed;
  quint64 m_pollsSkipped;
  quint64 m_consecutiveFailures;

  QTimer m_pollTimer;
  AsyncTcpDial m_tcpDial;
  QTcpSocket* m_tcpSocket;
  QUdpSocket* m_udpSocket;
  QWebSocket* m_webSocket;
  QNetworkAccessManager* m_httpManager;
  QPointer<QNetworkReply> m_reply;
  QByteArray m_udpBuffer;
  QByteArray m_tcpPendingWrites;
};
}  // namespace Drivers
}  // namespace IO

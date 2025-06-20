/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <QHostInfo>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QByteArray>
#include <QHostAddress>
#include <QAbstractSocket>

#include "IO/HAL_Driver.h"

namespace IO
{
namespace Drivers
{
/**
 * @brief The Network class
 *
 * Serial Studio "driver" class to interact with UDP/TCP network ports.
 */
class Network : public HAL_Driver
{
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
  Q_PROPERTY(QAbstractSocket::SocketType socketType
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
  // clang-format on

signals:
  void portChanged();
  void addressChanged();
  void socketTypeChanged();
  void udpMulticastChanged();
  void lookupActiveChanged();

private:
  explicit Network();
  Network(Network &&) = delete;
  Network(const Network &) = delete;
  Network &operator=(Network &&) = delete;
  Network &operator=(const Network &) = delete;

public:
  static Network &instance();

  void close() override;

  [[nodiscard]] bool isOpen() const override;
  [[nodiscard]] bool isReadable() const override;
  [[nodiscard]] bool isWritable() const override;
  [[nodiscard]] bool configurationOk() const override;
  [[nodiscard]] quint64 write(const QByteArray &data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;

  [[nodiscard]] quint16 tcpPort() const;
  [[nodiscard]] quint16 udpLocalPort() const;
  [[nodiscard]] quint16 udpRemotePort() const;

  [[nodiscard]] bool udpMulticast() const;
  [[nodiscard]] bool lookupActive() const;
  [[nodiscard]] int socketTypeIndex() const;
  [[nodiscard]] QAbstractSocket::SocketType socketType() const;

  [[nodiscard]] QTcpSocket *tcpSocket() { return &m_tcpSocket; }
  [[nodiscard]] QUdpSocket *udpSocket() { return &m_udpSocket; }

  [[nodiscard]] const QString &remoteAddress() const;
  [[nodiscard]] QStringList socketTypes() const;

  static quint16 defaultTcpPort() { return 23; }
  static quint16 defaultUdpLocalPort() { return 0; }
  static quint16 defaultUdpRemotePort() { return 53; }
  static const QString &defaultAddress()
  {
    static QString addr = QStringLiteral("127.0.0.1");
    return addr;
  }

public slots:
  void setTcpSocket();
  void setUdpSocket();
  void lookup(const QString &host);
  void setTcpPort(const quint16 port);
  void setUdpLocalPort(const quint16 port);
  void setUdpMulticast(const bool enabled);
  void setSocketTypeIndex(const int index);
  void setUdpRemotePort(const quint16 port);
  void setRemoteAddress(const QString &address);
  void setSocketType(const QAbstractSocket::SocketType type);

private slots:
  void onReadyRead();
  void lookupFinished(const QHostInfo &info);
  void onErrorOccurred(const QAbstractSocket::SocketError socketError);

private:
  QString m_address;
  quint16 m_tcpPort;
  bool m_hostExists;
  bool m_udpMulticast;
  bool m_lookupActive;
  quint16 m_udpLocalPort;
  quint16 m_udpRemotePort;
  QAbstractSocket::SocketType m_socketType;

  QTcpSocket m_tcpSocket;
  QUdpSocket m_udpSocket;
};
} // namespace Drivers
} // namespace IO

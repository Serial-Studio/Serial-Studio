/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QByteArray>
#include <QCanBusDevice>
#include <QCanBusFrame>
#include <QList>
#include <QSerialPort>
#include <QString>

namespace IO {
namespace Drivers {

/**
 * @brief Everything a serial CAN adapter backend does that is not its wire protocol: the port,
 *        the open and close sequences, the bounded receive buffer and the unplug handling.
 *        Subclasses supply the protocol alone, so a dead adapter reports the same way whichever
 *        one is speaking.
 */
class SerialCanBackendBase : public QCanBusDevice {
  Q_OBJECT

public:
  SerialCanBackendBase(SerialCanBackendBase&&)                 = delete;
  SerialCanBackendBase(const SerialCanBackendBase&)            = delete;
  SerialCanBackendBase& operator=(SerialCanBackendBase&&)      = delete;
  SerialCanBackendBase& operator=(const SerialCanBackendBase&) = delete;

  ~SerialCanBackendBase() override;

  [[nodiscard]] static qint64 arrivalMicroseconds();
  [[nodiscard]] static bool isFatalSerialError(QSerialPort::SerialPortError error) noexcept;
  static void appendBounded(QByteArray& buffer, const QByteArray& chunk, quint64& drops);

  [[nodiscard]] quint64 bufferOverflows() const noexcept;

protected:
  explicit SerialCanBackendBase(const QString& portName, qint32 baudRate, QObject* parent);

  [[nodiscard]] bool open() override;
  void close() override;

  [[nodiscard]] bool portIsOpen() const;
  [[nodiscard]] bool writeToPort(const QByteArray& bytes);
  [[nodiscard]] quint32 requestedBitrate() const;

  [[nodiscard]] virtual bool validateBitrate(quint32 bitrate, QString& reason) const = 0;
  [[nodiscard]] virtual bool sendInit(quint32 bitrate)                               = 0;
  [[nodiscard]] virtual bool openReplyIsError(const QByteArray& reply) const;
  virtual void drainBuffer(QByteArray& buffer, QList<QCanBusFrame>& frames) = 0;

  virtual void sendShutdown() {}

private slots:
  void onReadyRead();
  void onSerialError(QSerialPort::SerialPortError error);

private:
  void releasePort();

private:
  qint32 m_baudRate;
  quint64 m_bufferOverflows;
  QString m_portName;
  QSerialPort* m_port;
  QByteArray m_rxBuffer;
};
}  // namespace Drivers
}  // namespace IO

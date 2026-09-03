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

#include "IO/Drivers/CANBus/SerialCanBackendBase.h"

#include <chrono>
#include <QVariant>

static constexpr int kOpenAckTimeoutMs      = 250;
static constexpr quint32 kDefaultBitrate    = 500000;
static constexpr qsizetype kMaxRxBufferSize = 65536;

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the backend to a serial port name and the line speed its adapter family runs at.
 */
IO::Drivers::SerialCanBackendBase::SerialCanBackendBase(const QString& portName,
                                                        qint32 baudRate,
                                                        QObject* parent)
  : QCanBusDevice(parent)
  , m_baudRate(baudRate)
  , m_bufferOverflows(0)
  , m_portName(portName)
  , m_port(nullptr)
{}

/**
 * @brief Closes the channel if the subclass has not already done so.
 */
IO::Drivers::SerialCanBackendBase::~SerialCanBackendBase()
{
  if (state() != QCanBusDevice::UnconnectedState)
    close();
}

//--------------------------------------------------------------------------------------------------
// Shared helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief The capture time of the bytes just read, stamped at the read boundary because the adapter
 *        gives no timestamp of its own (source owns time).
 */
qint64 IO::Drivers::SerialCanBackendBase::arrivalMicroseconds()
{
  return std::chrono::duration_cast<std::chrono::microseconds>(
           std::chrono::steady_clock::now().time_since_epoch())
    .count();
}

/**
 * @brief Whether a serial error means the link is gone. An unplugged adapter raises ResourceError
 *        and then stops delivering readyRead: without this the state stayed Connected and the
 *        toolbar showed a live link on a dead bus for the rest of the session.
 */
bool IO::Drivers::SerialCanBackendBase::isFatalSerialError(
  QSerialPort::SerialPortError error) noexcept
{
  switch (error) {
    case QSerialPort::ResourceError:
    case QSerialPort::DeviceNotFoundError:
    case QSerialPort::PermissionError:
    case QSerialPort::UnsupportedOperationError:
      return true;
    default:
      return false;
  }
}

/**
 * @brief Appends @p chunk to @p buffer under a fixed ceiling. An adapter that never emits a frame
 *        terminator would otherwise grow the buffer without bound; the whole buffer is dropped and
 *        counted instead, because half a packet decodes into noise.
 */
void IO::Drivers::SerialCanBackendBase::appendBounded(QByteArray& buffer,
                                                      const QByteArray& chunk,
                                                      quint64& drops)
{
  if (chunk.size() > kMaxRxBufferSize) {
    buffer.clear();
    ++drops;
    return;
  }

  if (buffer.size() + chunk.size() > kMaxRxBufferSize) {
    buffer.clear();
    ++drops;
  }

  buffer.append(chunk);
}

/**
 * @brief How many times the receive buffer was dropped for overrunning its ceiling; polled, never
 *        pushed (spec 0033).
 */
quint64 IO::Drivers::SerialCanBackendBase::bufferOverflows() const noexcept
{
  return m_bufferOverflows;
}

/**
 * @brief Whether the port is usable for reads and writes.
 */
bool IO::Drivers::SerialCanBackendBase::portIsOpen() const
{
  return m_port != nullptr && m_port->isOpen();
}

/**
 * @brief Writes @p bytes to the port and flushes them, reporting a short write as a failure.
 */
bool IO::Drivers::SerialCanBackendBase::writeToPort(const QByteArray& bytes)
{
  if (!portIsOpen())
    return false;

  if (m_port->write(bytes) != bytes.size())
    return false;

  m_port->flush();
  return true;
}

/**
 * @brief The bitrate the driver configured, falling back to the family default when unset.
 */
quint32 IO::Drivers::SerialCanBackendBase::requestedBitrate() const
{
  const auto requested = configurationParameter(QCanBusDevice::BitRateKey).toUInt();
  return requested == 0 ? kDefaultBitrate : requested;
}

/**
 * @brief Whether the adapter's reply to the init sequence is a refusal; adapters that answer
 *        nothing are the common case, so the default accepts silence.
 */
bool IO::Drivers::SerialCanBackendBase::openReplyIsError(const QByteArray& reply) const
{
  Q_UNUSED(reply)
  return false;
}

//--------------------------------------------------------------------------------------------------
// QCanBusDevice interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the port and puts the adapter on the bus. The verdict reads the adapter's answer
 *        rather than the byte count of the init write: a refused bitrate used to look like a
 *        successful connect until the first frame never arrived.
 */
bool IO::Drivers::SerialCanBackendBase::open()
{
  QString reason;
  const quint32 bitrate = requestedBitrate();
  if (!validateBitrate(bitrate, reason)) {
    setError(reason, QCanBusDevice::ConfigurationError);
    setState(QCanBusDevice::UnconnectedState);
    return false;
  }

  m_rxBuffer.clear();
  m_port = new QSerialPort(m_portName, this);
  m_port->setBaudRate(m_baudRate);

  if (!m_port->open(QIODevice::ReadWrite)) {
    setError(tr("Could not open serial port %1: %2").arg(m_portName, m_port->errorString()),
             QCanBusDevice::ConnectionError);
    releasePort();
    setState(QCanBusDevice::UnconnectedState);
    return false;
  }

  connect(m_port, &QSerialPort::readyRead, this, &SerialCanBackendBase::onReadyRead);
  connect(m_port, &QSerialPort::errorOccurred, this, &SerialCanBackendBase::onSerialError);

  if (!sendInit(bitrate)) {
    setError(tr("The adapter on %1 did not accept the initialization sequence.").arg(m_portName),
             QCanBusDevice::ConnectionError);
    releasePort();
    setState(QCanBusDevice::UnconnectedState);
    return false;
  }

  if (m_port->waitForReadyRead(kOpenAckTimeoutMs)) {
    const QByteArray reply = m_port->readAll();
    if (openReplyIsError(reply)) {
      setError(tr("The adapter on %1 refused the requested configuration.").arg(m_portName),
               QCanBusDevice::ConnectionError);
      releasePort();
      setState(QCanBusDevice::UnconnectedState);
      return false;
    }

    appendBounded(m_rxBuffer, reply, m_bufferOverflows);
  }

  setState(QCanBusDevice::ConnectedState);
  return true;
}

/**
 * @brief Takes the adapter off the bus and releases the port. Safe on an already closed channel,
 *        which is what lets the error path and the destructor share it.
 */
void IO::Drivers::SerialCanBackendBase::close()
{
  setState(QCanBusDevice::ClosingState);

  if (portIsOpen())
    sendShutdown();

  releasePort();
  m_rxBuffer.clear();
  setState(QCanBusDevice::UnconnectedState);
}

//--------------------------------------------------------------------------------------------------
// Private slots & helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Buffers whatever arrived and hands the subclass's decoder every complete packet in it.
 */
void IO::Drivers::SerialCanBackendBase::onReadyRead()
{
  if (!portIsOpen())
    return;

  appendBounded(m_rxBuffer, m_port->readAll(), m_bufferOverflows);

  QList<QCanBusFrame> received;
  drainBuffer(m_rxBuffer, received);

  if (!received.isEmpty())
    enqueueReceivedFrames(received);
}

/**
 * @brief Reports a link the adapter lost. The channel is closed here rather than left Connected:
 *        the CANBus driver watches the state transition, and a state that never moves is a
 *        toolbar reading "connected" on a bus nothing can reach.
 */
void IO::Drivers::SerialCanBackendBase::onSerialError(QSerialPort::SerialPortError error)
{
  if (!isFatalSerialError(error))
    return;

  const QString reason = m_port ? m_port->errorString() : tr("The serial adapter is gone");
  setError(tr("Lost the adapter on %1: %2").arg(m_portName, reason), QCanBusDevice::ReadError);
  close();
}

/**
 * @brief Drops the port and its handlers. deleteLater() rather than delete: a run-loop source
 *        scheduled for the port still fires after close() and must find a live object.
 */
void IO::Drivers::SerialCanBackendBase::releasePort()
{
  if (m_port == nullptr)
    return;

  m_port->disconnect(this);
  if (m_port->isOpen())
    m_port->close();

  m_port->deleteLater();
  m_port = nullptr;
}

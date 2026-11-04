/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "IO/Drivers/UART/UartSequencePort.h"

#include <algorithm>

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

#include "Core/SSAssert.h"
#include "IO/Drivers/UART/UartPolicy.h"

/**
 * @brief Binds the open port.
 */
IO::Drivers::UartSequencePort::UartSequencePort(QSerialPort* port) : m_port(port)
{
  SS_ASSERT_LOG(port != nullptr);
  SS_ASSERT_LOG(port->isOpen());
}

/**
 * @brief Discards what the port has buffered on input.
 */
bool IO::Drivers::UartSequencePort::clearInput()
{
  return portReady() && m_port->clear(QSerialPort::Input);
}

/**
 * @brief Changes the running baud rate, never the saved one.
 */
bool IO::Drivers::UartSequencePort::setBaud(qint32 rate)
{
  return portReady() && applyBaud(*m_port, rate);
}

/**
 * @brief Asserts or releases BREAK. Success means the port accepted the request; it says nothing
 *        about how long an adapter actually holds the line.
 */
bool IO::Drivers::UartSequencePort::setBreak(bool enabled)
{
  return portReady() && m_port->setBreakEnabled(enabled);
}

/**
 * @brief The cause of the last refusal: this adapter's own, else the port's.
 */
QString IO::Drivers::UartSequencePort::errorString() const
{
  if (!m_error.isEmpty() || m_port.isNull())
    return m_error;

  return m_port->errorString();
}

/**
 * @brief Queues @p data on the port; -1 when it is closed.
 */
qint64 IO::Drivers::UartSequencePort::write(const QByteArray& data)
{
  if (!portReady())
    return -1;

  return m_port->write(data);
}

/**
 * @brief Reads the modem lines of the open port.
 */
QSerialPort::PinoutSignals IO::Drivers::UartSequencePort::lineStates() const
{
  if (m_port.isNull() || !m_port->isOpen())
    return QSerialPort::NoSignal;

  return m_port->pinoutSignals();
}

/**
 * @brief Changes the running flow-control mode.
 */
bool IO::Drivers::UartSequencePort::setFlowControl(QSerialPort::FlowControl mode)
{
  return portReady() && m_port->setFlowControl(mode);
}

/**
 * @brief Drives DTR or RTS. RTS is refused under RTS/CTS flow control, where the port driver owns
 *        it and Qt would answer UnsupportedOperationError.
 */
bool IO::Drivers::UartSequencePort::setLine(IO::SequenceStep::Line line, bool high)
{
  if (!portReady())
    return false;

  if (line == IO::SequenceStep::Line::Dtr)
    return m_port->setDataTerminalReady(high);

  if (!UartPolicy::rtsSettable(m_port->flowControl()))
    return refuse(tr("RTS is driven by RTS/CTS flow control"));

  return m_port->setRequestToSend(high);
}

//--------------------------------------------------------------------------------------------------
// Buffer sizing, shared with the driver's open()
//--------------------------------------------------------------------------------------------------

/**
 * @brief Calculates an ideal read buffer size for a serial port.
 */
std::size_t IO::Drivers::UartSequencePort::readBufferSize(qint32 baud)
{
  std::size_t bytes = static_cast<std::size_t>(baud * 0.02);
  bytes             = std::max<std::size_t>(256, bytes);
  bytes             = std::min<std::size_t>(524288, bytes);

  constexpr std::size_t granularity = 256;
  bytes                             = ((bytes + granularity - 1) / granularity) * granularity;

  return bytes;
}

/**
 * @brief Sets the running rate of an open port and resizes its buffers to match.
 */
bool IO::Drivers::UartSequencePort::applyBaud(QSerialPort& port, qint32 rate)
{
  if (!port.isOpen() || rate < 1 || !port.setBaudRate(rate))
    return false;

  port.setReadBufferSize(static_cast<qint64>(readBufferSize(rate)));
  tuneNativeBuffer(&port, rate);
  return true;
}

/**
 * @brief Expands the kernel-side serial FIFO and tunes COMM timeouts for high-throughput
 *        reception on Windows (a no-op elsewhere). setReadBufferSize() only sizes Qt's ring;
 *        the OS driver's own FIFO overflows at high baud rates if left at its default.
 */
void IO::Drivers::UartSequencePort::tuneNativeBuffer(QSerialPort* port, qint32 baud)
{
#ifdef Q_OS_WIN
  if (!port || !port->isOpen())
    return;

  const auto handle = port->handle();
  if (handle == INVALID_HANDLE_VALUE)
    return;

  const std::size_t buf = readBufferSize(baud);
  SetupComm(handle, static_cast<DWORD>(buf), static_cast<DWORD>(buf));

  COMMTIMEOUTS timeouts{};
  if (!GetCommTimeouts(handle, &timeouts))
    return;

  timeouts.ReadIntervalTimeout        = MAXDWORD;
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.ReadTotalTimeoutConstant   = 0;
  SetCommTimeouts(handle, &timeouts);
#else
  Q_UNUSED(port)
  Q_UNUSED(baud)
#endif
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Clears the last refusal and reports whether the port is still open.
 */
bool IO::Drivers::UartSequencePort::portReady()
{
  m_error.clear();
  if (!m_port.isNull() && m_port->isOpen())
    return true;

  return refuse(tr("the port is closed"));
}

/**
 * @brief Records why the adapter refused an operation; always false.
 */
bool IO::Drivers::UartSequencePort::refuse(const QString& reason)
{
  m_error = reason;
  return false;
}

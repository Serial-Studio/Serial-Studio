/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#include "IO/Drivers/SlcanBackend.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVariant>

//--------------------------------------------------------------------------------------------------
// slcan / LAWICEL ASCII protocol helpers
//--------------------------------------------------------------------------------------------------

// Serial line speed; USB-CDC slcan adapters ignore it, USBtin expects 115200
constexpr qint32 kSlcanBaudRate = 115200;

// LAWICEL bitrate command index Sn, ordered by ascending bitrate
constexpr std::uint32_t kSlcanBitrates[] = {
  10000, 20000, 50000, 100000, 125000, 250000, 500000, 800000, 1000000};

/**
 * @brief Maps a bitrate in bits/s to its LAWICEL "Sn" index, or -1 when unsupported.
 */
[[nodiscard]] static int slcanBitrateIndex(std::uint32_t bitrate)
{
  for (std::size_t i = 0; i < std::size(kSlcanBitrates); ++i)
    if (kSlcanBitrates[i] == bitrate)
      return static_cast<int>(i);

  return -1;
}

/**
 * @brief Decodes a single LAWICEL frame token (t/T/r/R) into a QCanBusFrame.
 */
[[nodiscard]] static bool parseSlcanToken(const QByteArray& token, QCanBusFrame& out)
{
  if (token.isEmpty())
    return false;

  const char type     = token.at(0);
  const bool extended = (type == 'T' || type == 'R');
  const bool remote   = (type == 'r' || type == 'R');
  if (type != 't' && type != 'T' && type != 'r' && type != 'R')
    return false;

  const int idLen = extended ? 8 : 3;
  if (token.size() < 1 + idLen + 1)
    return false;

  bool ok                = false;
  const std::uint32_t id = token.mid(1, idLen).toUInt(&ok, 16);
  const int dlc          = QByteArray(1, token.at(1 + idLen)).toInt(&ok, 16);
  if (!ok || dlc < 0 || dlc > 8)
    return false;

  QByteArray payload;
  if (!remote) {
    const QByteArray hex = token.mid(1 + idLen + 1, dlc * 2);
    if (hex.size() < dlc * 2)
      return false;

    payload = QByteArray::fromHex(hex);
  }

  out = QCanBusFrame(id, payload);
  out.setExtendedFrameFormat(extended);
  if (remote)
    out.setFrameType(QCanBusFrame::RemoteRequestFrame);

  return true;
}

//--------------------------------------------------------------------------------------------------
// Static plugin identity & enumeration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns this backend's registry entry.
 */
IO::Drivers::CanBackends::Entry IO::Drivers::SlcanBackend::registration()
{
  return {pluginKey(), QStringLiteral("Serial CAN"), true, &availableInterfaces, &create};
}

/**
 * @brief Returns the synthetic plugin key for slcan adapters.
 */
const QString& IO::Drivers::SlcanBackend::pluginKey()
{
  static const QString key = QStringLiteral("slcan");
  return key;
}

/**
 * @brief Returns the names of every available serial port.
 */
QStringList IO::Drivers::SlcanBackend::availableInterfaces()
{
  QStringList ports;
  for (const QSerialPortInfo& info : QSerialPortInfo::availablePorts())
    ports.append(info.portName());

  return ports;
}

/**
 * @brief Factory used by the CAN backend registry.
 */
QCanBusDevice* IO::Drivers::SlcanBackend::create(const QString& portName)
{
  return new SlcanBackend(portName);
}

//--------------------------------------------------------------------------------------------------
// Constructor/destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the backend bound to a serial port name.
 */
IO::Drivers::SlcanBackend::SlcanBackend(const QString& portName, QObject* parent)
  : QCanBusDevice(parent), m_port(nullptr), m_portName(portName)
{}

/**
 * @brief Closes the serial port and releases the channel.
 */
IO::Drivers::SlcanBackend::~SlcanBackend()
{
  if (state() != QCanBusDevice::UnconnectedState)
    close();
}

//--------------------------------------------------------------------------------------------------
// QCanBusDevice interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the serial port and starts the slcan channel at the configured bitrate.
 */
bool IO::Drivers::SlcanBackend::open()
{
  const auto requested = configurationParameter(QCanBusDevice::BitRateKey).toUInt();
  const int index      = slcanBitrateIndex(requested == 0 ? 500000 : requested);
  if (index < 0) {
    setError(tr("The bitrate %1 bps is not a standard slcan rate.").arg(requested),
             QCanBusDevice::ConfigurationError);
    setState(QCanBusDevice::UnconnectedState);
    return false;
  }

  m_port = new QSerialPort(m_portName, this);
  m_port->setBaudRate(kSlcanBaudRate);

  if (!m_port->open(QIODevice::ReadWrite)) {
    setError(tr("Could not open serial port %1: %2").arg(m_portName, m_port->errorString()),
             QCanBusDevice::ConnectionError);
    m_port->deleteLater();
    m_port = nullptr;
    setState(QCanBusDevice::UnconnectedState);
    return false;
  }

  connect(m_port, &QSerialPort::readyRead, this, &SlcanBackend::onReadyRead);

  (void)sendCommand(QByteArrayLiteral("C\r"));
  (void)sendCommand("S" + QByteArray::number(index) + "\r");
  if (!sendCommand(QByteArrayLiteral("O\r"))) {
    setError(tr("Failed to open the slcan channel."), QCanBusDevice::ConnectionError);
    m_port->close();
    m_port->deleteLater();
    m_port = nullptr;
    setState(QCanBusDevice::UnconnectedState);
    return false;
  }

  setState(QCanBusDevice::ConnectedState);
  return true;
}

/**
 * @brief Closes the slcan channel and the serial port.
 */
void IO::Drivers::SlcanBackend::close()
{
  setState(QCanBusDevice::ClosingState);

  if (m_port) {
    if (m_port->isOpen()) {
      (void)sendCommand(QByteArrayLiteral("C\r"));
      m_port->close();
    }

    m_port->deleteLater();
    m_port = nullptr;
  }

  m_rxBuffer.clear();
  setState(QCanBusDevice::UnconnectedState);
}

/**
 * @brief Encodes a QCanBusFrame as a LAWICEL command and writes it to the port.
 */
bool IO::Drivers::SlcanBackend::writeFrame(const QCanBusFrame& frame)
{
  if (!m_port || !m_port->isOpen()) {
    setError(tr("slcan adapter is not open for writing."), QCanBusDevice::WriteError);
    return false;
  }

  if (!frame.isValid())
    return false;

  const bool extended = frame.hasExtendedFrameFormat();
  const bool remote   = frame.frameType() == QCanBusFrame::RemoteRequestFrame;
  const int idWidth   = extended ? 8 : 3;

  char prefix = extended ? 'T' : 't';
  if (remote)
    prefix = extended ? 'R' : 'r';

  const QByteArray payload = frame.payload();
  const int dlc            = qMin(payload.size(), 8);

  QByteArray command(1, prefix);
  command += QByteArray::number(frame.frameId(), 16).toUpper().rightJustified(idWidth, '0');
  command += QByteArray::number(dlc, 16).toUpper();
  if (!remote)
    command += payload.left(dlc).toHex().toUpper();

  command += '\r';
  if (!sendCommand(command))
    return false;

  Q_EMIT framesWritten(1);
  return true;
}

/**
 * @brief Produces a human-readable description for a CAN error frame.
 */
QString IO::Drivers::SlcanBackend::interpretErrorFrame(const QCanBusFrame& frame)
{
  Q_UNUSED(frame)
  return tr("CAN bus error reported by the slcan adapter.");
}

//--------------------------------------------------------------------------------------------------
// Private slots & helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reassembles CR-terminated slcan tokens and enqueues decoded frames.
 */
void IO::Drivers::SlcanBackend::onReadyRead()
{
  if (!m_port)
    return;

  // code-verify off
  // Driver acquisition path (not the Dashboard draw hotpath): byte accumulation and
  // frame batching allocate by nature, as in every driver read callback.
  m_rxBuffer.append(m_port->readAll());

  QList<QCanBusFrame> received;
  int end = m_rxBuffer.indexOf('\r');
  while (end >= 0) {
    const QByteArray token = m_rxBuffer.left(end);
    m_rxBuffer.remove(0, end + 1);

    QCanBusFrame frame;
    if (parseSlcanToken(token, frame))
      received.append(frame);

    end = m_rxBuffer.indexOf('\r');
  }
  // code-verify on

  if (!received.isEmpty())
    enqueueReceivedFrames(received);
}

/**
 * @brief Writes a raw command to the serial port and flushes it.
 */
bool IO::Drivers::SlcanBackend::sendCommand(const QByteArray& command)
{
  if (!m_port || !m_port->isOpen())
    return false;

  if (m_port->write(command) != command.size())
    return false;

  m_port->flush();
  return true;
}

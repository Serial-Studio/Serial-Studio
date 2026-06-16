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

#include "IO/Drivers/SeeedCanBackend.h"

#include <chrono>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVariant>

//--------------------------------------------------------------------------------------------------
// Seeed / Waveshare USB-CAN Analyzer serial protocol helpers
//--------------------------------------------------------------------------------------------------

// CH340-based analyzers run their serial link at 2 Mbaud
constexpr qint32 kSeeedBaudRate = 2000000;

// Variable-length data frame markers and type-byte bit layout
constexpr std::uint8_t kFrameStart  = 0xaa;
constexpr std::uint8_t kFrameEnd    = 0x55;
constexpr std::uint8_t kTypeBase    = 0xc0;
constexpr std::uint8_t kTypeExtBit  = 0x20;
constexpr std::uint8_t kTypeRtrBit  = 0x10;
constexpr std::uint8_t kTypeDlcMask = 0x0f;

/**
 * @enum SeeedParse
 * @brief Outcome of decoding one packet from the receive buffer.
 */
enum class SeeedParse {
  Frame,
  Resync,
  NeedMore,
};

/**
 * @brief Maps a bitrate in bits/s to the analyzer's configuration code, or 0 when unsupported.
 */
[[nodiscard]] static std::uint8_t seeedBitrateCode(std::uint32_t bitrate)
{
  switch (bitrate) {
    case 1000000:
      return 0x01;
    case 800000:
      return 0x02;
    case 500000:
      return 0x03;
    case 250000:
      return 0x05;
    case 125000:
      return 0x07;
    case 100000:
      return 0x08;
    case 50000:
      return 0x09;
    case 20000:
      return 0x0a;
    case 10000:
      return 0x0b;
    default:
      return 0x00;
  }
}

/**
 * @brief Decodes one variable-length packet at the front of @p buf (which must start with 0xAA).
 */
[[nodiscard]] static SeeedParse decodeSeeedFrame(const QByteArray& buf,
                                                 QCanBusFrame& out,
                                                 int& consumed)
{
  if (buf.size() < 2)
    return SeeedParse::NeedMore;

  const std::uint8_t typeByte = static_cast<std::uint8_t>(buf.at(1));

  if (typeByte == kFrameEnd) {
    if (buf.size() < 20)
      return SeeedParse::NeedMore;

    consumed = 20;
    return SeeedParse::Resync;
  }

  const int dlc = typeByte & kTypeDlcMask;
  if ((typeByte & kTypeBase) != kTypeBase || dlc > 8) {
    consumed = 1;
    return SeeedParse::Resync;
  }

  const bool extended = (typeByte & kTypeExtBit) != 0;
  const int idLen     = extended ? 4 : 2;
  const int total     = 2 + idLen + dlc + 1;
  if (buf.size() < total)
    return SeeedParse::NeedMore;

  if (static_cast<std::uint8_t>(buf.at(total - 1)) != kFrameEnd) {
    consumed = 1;
    return SeeedParse::Resync;
  }

  std::uint32_t id = 0;
  for (int i = 0; i < idLen; ++i)
    id |= static_cast<std::uint32_t>(static_cast<std::uint8_t>(buf.at(2 + i))) << (8 * i);

  out = QCanBusFrame(id, buf.mid(2 + idLen, dlc));
  out.setExtendedFrameFormat(extended);
  if (typeByte & kTypeRtrBit)
    out.setFrameType(QCanBusFrame::RemoteRequestFrame);

  consumed = total;
  return SeeedParse::Frame;
}

//--------------------------------------------------------------------------------------------------
// Static plugin identity & enumeration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns this backend's registry entry.
 */
IO::Drivers::CanBackends::Entry IO::Drivers::SeeedCanBackend::registration()
{
  return {pluginKey(), QStringLiteral("Seeed / Waveshare"), true, &availableInterfaces, &create};
}

/**
 * @brief Returns the synthetic plugin key for the USB-CAN Analyzer.
 */
const QString& IO::Drivers::SeeedCanBackend::pluginKey()
{
  static const QString key = QStringLiteral("seeed_usbcan");
  return key;
}

/**
 * @brief Returns the names of every available serial port.
 */
QStringList IO::Drivers::SeeedCanBackend::availableInterfaces()
{
  QStringList ports;
  for (const QSerialPortInfo& info : QSerialPortInfo::availablePorts())
    ports.append(info.portName());

  return ports;
}

/**
 * @brief Factory used by the CAN backend registry.
 */
QCanBusDevice* IO::Drivers::SeeedCanBackend::create(const QString& portName)
{
  return new SeeedCanBackend(portName);
}

//--------------------------------------------------------------------------------------------------
// Constructor/destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the backend bound to a serial port name.
 */
IO::Drivers::SeeedCanBackend::SeeedCanBackend(const QString& portName, QObject* parent)
  : QCanBusDevice(parent), m_port(nullptr), m_portName(portName)
{}

/**
 * @brief Closes the serial port and releases the channel.
 */
IO::Drivers::SeeedCanBackend::~SeeedCanBackend()
{
  if (state() != QCanBusDevice::UnconnectedState)
    close();
}

//--------------------------------------------------------------------------------------------------
// QCanBusDevice interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the serial port and sends the analyzer initialization frame.
 */
bool IO::Drivers::SeeedCanBackend::open()
{
  const auto requested = configurationParameter(QCanBusDevice::BitRateKey).toUInt();
  if (seeedBitrateCode(requested == 0 ? 500000 : requested) == 0) {
    setError(tr("The bitrate %1 bps is not supported by the USB-CAN Analyzer.").arg(requested),
             QCanBusDevice::ConfigurationError);
    setState(QCanBusDevice::UnconnectedState);
    return false;
  }

  m_port = new QSerialPort(m_portName, this);
  m_port->setBaudRate(kSeeedBaudRate);

  if (!m_port->open(QIODevice::ReadWrite)) {
    setError(tr("Could not open serial port %1: %2").arg(m_portName, m_port->errorString()),
             QCanBusDevice::ConnectionError);
    m_port->deleteLater();
    m_port = nullptr;
    setState(QCanBusDevice::UnconnectedState);
    return false;
  }

  connect(m_port, &QSerialPort::readyRead, this, &SeeedCanBackend::onReadyRead);

  if (!sendInitFrame(requested == 0 ? 500000 : requested)) {
    setError(tr("Failed to initialize the USB-CAN Analyzer."), QCanBusDevice::ConnectionError);
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
 * @brief Closes the serial port.
 */
void IO::Drivers::SeeedCanBackend::close()
{
  setState(QCanBusDevice::ClosingState);

  if (m_port) {
    if (m_port->isOpen())
      m_port->close();

    m_port->deleteLater();
    m_port = nullptr;
  }

  m_rxBuffer.clear();
  setState(QCanBusDevice::UnconnectedState);
}

/**
 * @brief Encodes a QCanBusFrame as a variable-length analyzer packet and writes it.
 */
bool IO::Drivers::SeeedCanBackend::writeFrame(const QCanBusFrame& frame)
{
  if (!m_port || !m_port->isOpen()) {
    setError(tr("USB-CAN Analyzer is not open for writing."), QCanBusDevice::WriteError);
    return false;
  }

  if (!frame.isValid())
    return false;

  const bool extended = frame.hasExtendedFrameFormat();
  const bool remote   = frame.frameType() == QCanBusFrame::RemoteRequestFrame;
  const int idLen     = extended ? 4 : 2;

  const QByteArray payload = frame.payload();
  const int dlc            = qMin(payload.size(), 8);

  std::uint8_t typeByte = kTypeBase | static_cast<std::uint8_t>(dlc);
  if (extended)
    typeByte |= kTypeExtBit;

  if (remote)
    typeByte |= kTypeRtrBit;

  QByteArray packet;
  packet.append(static_cast<char>(kFrameStart));
  packet.append(static_cast<char>(typeByte));

  const std::uint32_t id = frame.frameId();
  for (int i = 0; i < idLen; ++i)
    packet.append(static_cast<char>((id >> (8 * i)) & 0xff));

  packet.append(payload.left(dlc));
  packet.append(static_cast<char>(kFrameEnd));

  if (m_port->write(packet) != packet.size())
    return false;

  m_port->flush();
  Q_EMIT framesWritten(1);
  return true;
}

/**
 * @brief Produces a human-readable description for a CAN error frame.
 */
QString IO::Drivers::SeeedCanBackend::interpretErrorFrame(const QCanBusFrame& frame)
{
  Q_UNUSED(frame)
  return tr("CAN bus error reported by the USB-CAN Analyzer.");
}

//--------------------------------------------------------------------------------------------------
// Private slots & helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reassembles variable-length analyzer packets and enqueues decoded frames.
 */
void IO::Drivers::SeeedCanBackend::onReadyRead()
{
  if (!m_port)
    return;

  // code-verify off
  // Driver acquisition path (not the Dashboard draw hotpath): byte accumulation and
  // frame batching allocate by nature, as in every driver read callback. The drain
  // loop is bounded -- each pass consumes >= 1 byte or breaks on NeedMore.
  m_rxBuffer.append(m_port->readAll());

  const qint64 arrivalUsec = std::chrono::duration_cast<std::chrono::microseconds>(
                               std::chrono::steady_clock::now().time_since_epoch())
                               .count();

  QList<QCanBusFrame> received;
  while (true) {
    const int start = m_rxBuffer.indexOf(static_cast<char>(kFrameStart));
    if (start < 0) {
      m_rxBuffer.clear();
      break;
    }

    if (start > 0)
      m_rxBuffer.remove(0, start);

    int consumed = 0;
    QCanBusFrame frame;
    const SeeedParse result = decodeSeeedFrame(m_rxBuffer, frame, consumed);
    if (result == SeeedParse::NeedMore)
      break;

    if (result == SeeedParse::Frame) {
      frame.setTimeStamp(QCanBusFrame::TimeStamp::fromMicroSeconds(arrivalUsec));
      received.append(frame);
    }

    m_rxBuffer.remove(0, consumed);
  }
  // code-verify on

  if (!received.isEmpty())
    enqueueReceivedFrames(received);
}

/**
 * @brief Builds and sends the 20-byte initialization frame for the given bitrate.
 */
bool IO::Drivers::SeeedCanBackend::sendInitFrame(quint32 bitrate)
{
  QByteArray frame(20, 0);
  frame[0]  = static_cast<char>(kFrameStart);
  frame[1]  = static_cast<char>(kFrameEnd);
  frame[2]  = 0x12;
  frame[3]  = static_cast<char>(seeedBitrateCode(bitrate));
  frame[4]  = 0x01;
  frame[13] = 0x00;
  frame[14] = 0x01;

  int checksum = 0;
  for (int i = 2; i <= 18; ++i)
    checksum += static_cast<std::uint8_t>(frame.at(i));

  frame[19] = static_cast<char>(checksum & 0xff);

  if (m_port->write(frame) != frame.size())
    return false;

  m_port->flush();
  return true;
}

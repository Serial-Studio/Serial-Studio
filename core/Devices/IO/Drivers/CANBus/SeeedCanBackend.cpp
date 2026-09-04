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

#include "IO/Drivers/CANBus/SeeedCanBackend.h"

#include <cstdint>
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
 * @brief Maps a bitrate in bits/s to the analyzer's configuration code, or 0 when unsupported.
 */
std::uint8_t IO::Drivers::SeeedCanBackend::bitrateCode(quint32 bitrate)
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
 * @brief Decodes one variable-length packet at the front of @p buffer (which must start with 0xAA).
 */
IO::Drivers::SeeedCanBackend::Parse IO::Drivers::SeeedCanBackend::decodePacket(
  const QByteArray& buffer, QCanBusFrame& out, int& consumed)
{
  if (buffer.size() < 2)
    return Parse::NeedMore;

  const std::uint8_t typeByte = static_cast<std::uint8_t>(buffer.at(1));

  if (typeByte == kFrameEnd) {
    if (buffer.size() < 20)
      return Parse::NeedMore;

    consumed = 20;
    return Parse::Resync;
  }

  const int dlc = typeByte & kTypeDlcMask;
  if ((typeByte & kTypeBase) != kTypeBase || dlc > 8) {
    consumed = 1;
    return Parse::Resync;
  }

  const bool extended = (typeByte & kTypeExtBit) != 0;
  const int idLen     = extended ? 4 : 2;
  const int total     = 2 + idLen + dlc + 1;
  if (buffer.size() < total)
    return Parse::NeedMore;

  if (static_cast<std::uint8_t>(buffer.at(total - 1)) != kFrameEnd) {
    consumed = 1;
    return Parse::Resync;
  }

  std::uint32_t id = 0;
  for (int i = 0; i < idLen; ++i)
    id |= static_cast<std::uint32_t>(static_cast<std::uint8_t>(buffer.at(2 + i))) << (8 * i);

  out = QCanBusFrame(id, buffer.mid(2 + idLen, dlc));
  out.setExtendedFrameFormat(extended);
  if (typeByte & kTypeRtrBit)
    out.setFrameType(QCanBusFrame::RemoteRequestFrame);

  consumed = total;
  return Parse::Frame;
}

//--------------------------------------------------------------------------------------------------
// Static plugin identity & enumeration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns this backend's registry entry.
 */
IO::Drivers::CanBackends::Entry IO::Drivers::SeeedCanBackend::registration()
{
  return {
    pluginKey(), QStringLiteral("Seeed / Waveshare"), true, &availableInterfaces, &create, nullptr};
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
  : SerialCanBackendBase(portName, kSeeedBaudRate, parent)
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
 * @brief Rejects a bitrate the analyzer has no configuration code for.
 */
bool IO::Drivers::SeeedCanBackend::validateBitrate(quint32 bitrate, QString& reason) const
{
  if (bitrateCode(bitrate) != 0)
    return true;

  reason = tr("The bitrate %1 bps is not supported by the USB-CAN Analyzer.").arg(bitrate);
  return false;
}

/**
 * @brief Encodes a QCanBusFrame as a variable-length analyzer packet and writes it.
 */
bool IO::Drivers::SeeedCanBackend::writeFrame(const QCanBusFrame& frame)
{
  if (!portIsOpen()) {
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

  if (!writeToPort(packet))
    return false;

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
// Protocol hooks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reassembles variable-length analyzer packets into frames, consuming what it decoded.
 */
void IO::Drivers::SeeedCanBackend::drainBuffer(QByteArray& buffer, QList<QCanBusFrame>& frames)
{
  const qint64 arrivalUsec = arrivalMicroseconds();

  // code-verify off
  // Driver acquisition path (not the Dashboard draw hotpath): byte accumulation and
  // frame batching allocate by nature, as in every driver read callback. The drain
  // loop is bounded -- each pass consumes >= 1 byte or breaks on NeedMore.
  while (true) {
    const int start = buffer.indexOf(static_cast<char>(kFrameStart));
    if (start < 0) {
      buffer.clear();
      break;
    }

    if (start > 0)
      buffer.remove(0, start);

    int consumed = 0;
    QCanBusFrame frame;
    const Parse result = decodePacket(buffer, frame, consumed);
    if (result == Parse::NeedMore)
      break;

    if (result == Parse::Frame) {
      frame.setTimeStamp(QCanBusFrame::TimeStamp::fromMicroSeconds(arrivalUsec));
      frames.append(frame);
    }

    buffer.remove(0, consumed);
  }
  // code-verify on
}

/**
 * @brief Builds and sends the 20-byte initialization frame for the given bitrate.
 */
bool IO::Drivers::SeeedCanBackend::sendInit(quint32 bitrate)
{
  QByteArray frame(20, 0);
  frame[0]  = static_cast<char>(kFrameStart);
  frame[1]  = static_cast<char>(kFrameEnd);
  frame[2]  = 0x12;
  frame[3]  = static_cast<char>(bitrateCode(bitrate));
  frame[4]  = 0x01;
  frame[13] = 0x00;
  frame[14] = 0x01;

  int checksum = 0;
  for (int i = 2; i <= 18; ++i)
    checksum += static_cast<std::uint8_t>(frame.at(i));

  frame[19] = static_cast<char>(checksum & 0xff);

  return writeToPort(frame);
}

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

#include "IO/Drivers/CANBus/SlcanBackend.h"

#include <cstdint>
#include <iterator>
#include <QSerialPortInfo>
#include <QVariant>

//--------------------------------------------------------------------------------------------------
// slcan / LAWICEL ASCII protocol helpers
//--------------------------------------------------------------------------------------------------

// Serial line speed; USB-CDC slcan adapters ignore it, USBtin expects 115200
constexpr qint32 kSlcanBaudRate = 115200;

// The adapter answers a rejected command with BEL
constexpr char kSlcanBell = '\a';

// LAWICEL bitrate command index Sn, ordered by ascending bitrate
constexpr std::uint32_t kSlcanBitrates[] = {
  10000, 20000, 50000, 100000, 125000, 250000, 500000, 800000, 1000000};

//--------------------------------------------------------------------------------------------------
// Static plugin identity & enumeration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns this backend's registry entry.
 */
IO::Drivers::CanBackends::Entry IO::Drivers::SlcanBackend::registration()
{
  return {pluginKey(), QStringLiteral("Serial CAN"), true, &availableInterfaces, &create, nullptr};
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

/**
 * @brief Maps a bitrate in bits/s to its LAWICEL "Sn" index, or -1 when unsupported.
 */
int IO::Drivers::SlcanBackend::bitrateIndex(quint32 bitrate)
{
  for (std::size_t i = 0; i < std::size(kSlcanBitrates); ++i)
    if (kSlcanBitrates[i] == bitrate)
      return static_cast<int>(i);

  return -1;
}

/**
 * @brief Decodes a single LAWICEL frame token (t/T/r/R) into a QCanBusFrame. The identifier and
 *        the DLC carry SEPARATE conversion flags: sharing one let a non-hex identifier publish a
 *        fabricated frame on id 0x000 whenever the DLC digit happened to parse.
 */
bool IO::Drivers::SlcanBackend::parseToken(const QByteArray& token, QCanBusFrame& out)
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

  bool idOk              = false;
  bool dlcOk             = false;
  const std::uint32_t id = token.mid(1, idLen).toUInt(&idOk, 16);
  const int dlc          = QByteArray(1, token.at(1 + idLen)).toInt(&dlcOk, 16);
  if (!idOk || !dlcOk || dlc < 0 || dlc > 8)
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
// Constructor/destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the backend bound to a serial port name.
 */
IO::Drivers::SlcanBackend::SlcanBackend(const QString& portName, QObject* parent)
  : SerialCanBackendBase(portName, kSlcanBaudRate, parent)
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
// Protocol hooks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rejects a bitrate that has no LAWICEL command index.
 */
bool IO::Drivers::SlcanBackend::validateBitrate(quint32 bitrate, QString& reason) const
{
  if (bitrateIndex(bitrate) >= 0)
    return true;

  reason = tr("The bitrate %1 bps is not a standard slcan rate.").arg(bitrate);
  return false;
}

/**
 * @brief Closes any channel the adapter left open, selects the bitrate and opens the channel.
 */
bool IO::Drivers::SlcanBackend::sendInit(quint32 bitrate)
{
  const int index = bitrateIndex(bitrate);
  if (index < 0)
    return false;

  (void)writeToPort(QByteArrayLiteral("C\r"));
  (void)writeToPort("S" + QByteArray::number(index) + "\r");

  return writeToPort(QByteArrayLiteral("O\r"));
}

/**
 * @brief Takes the adapter off the bus.
 */
void IO::Drivers::SlcanBackend::sendShutdown()
{
  (void)writeToPort(QByteArrayLiteral("C\r"));
}

/**
 * @brief A BEL anywhere in the adapter's answer is its refusal of one of the open commands, and
 *        the only evidence a wrong bitrate leaves before the frames never arrive at all.
 */
bool IO::Drivers::SlcanBackend::openReplyIsError(const QByteArray& reply) const
{
  return reply.contains(kSlcanBell);
}

/**
 * @brief Reassembles CR-terminated slcan tokens into frames, consuming what it decoded.
 */
void IO::Drivers::SlcanBackend::drainBuffer(QByteArray& buffer, QList<QCanBusFrame>& frames)
{
  const qint64 arrivalUsec = arrivalMicroseconds();

  // code-verify off
  // Driver acquisition path (not the Dashboard draw hotpath): byte accumulation and
  // frame batching allocate by nature, as in every driver read callback.
  int end = buffer.indexOf('\r');
  while (end >= 0) {
    const QByteArray token = buffer.left(end);
    buffer.remove(0, end + 1);

    QCanBusFrame frame;
    if (parseToken(token, frame)) {
      frame.setTimeStamp(QCanBusFrame::TimeStamp::fromMicroSeconds(arrivalUsec));
      frames.append(frame);
    }

    end = buffer.indexOf('\r');
  }
  // code-verify on
}

//--------------------------------------------------------------------------------------------------
// QCanBusDevice interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Encodes a QCanBusFrame as a LAWICEL command and writes it to the port.
 */
bool IO::Drivers::SlcanBackend::writeFrame(const QCanBusFrame& frame)
{
  if (!portIsOpen()) {
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
  if (!writeToPort(command))
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

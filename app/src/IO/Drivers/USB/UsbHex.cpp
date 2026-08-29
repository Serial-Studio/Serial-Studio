/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

#include "IO/Drivers/USB/UsbHex.h"

#include <QObject>

/**
 * @brief True when @p c is a hexadecimal digit (0-9, a-f, A-F).
 */
bool IO::Drivers::UsbHex::isHexChar(const QChar c)
{
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

/**
 * @brief Parses a hex string (optional 0x prefix) into an unsigned value bounded by @p max.
 */
unsigned int IO::Drivers::UsbHex::parseHexUInt(const QString& text,
                                               const unsigned int max,
                                               bool& ok)
{
  QString cleaned = text.trimmed();
  if (cleaned.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive))
    cleaned.remove(0, 2);

  const unsigned int value = cleaned.toUInt(&ok, 16);
  if (!ok || value > max)
    ok = false;

  return value;
}

/**
 * @brief Parses a whitespace-tolerant hex byte string into raw bytes; ok=false on any non-hex.
 */
QByteArray IO::Drivers::UsbHex::parseHexBytes(const QString& text, bool& ok)
{
  QString cleaned;
  cleaned.reserve(text.size());

  for (const QChar c : text) {
    if (c.isSpace())
      continue;

    if (!isHexChar(c)) {
      ok = false;
      return {};
    }

    cleaned.append(c);
  }

  ok = (cleaned.size() % 2 == 0);
  return ok ? QByteArray::fromHex(cleaned.toLatin1()) : QByteArray{};
}

/**
 * @brief Maps a libusb transfer status to a short human-readable failure reason.
 */
QString IO::Drivers::UsbHex::controlStatusText(const int status)
{
  switch (status) {
    case kTransferTimedOut:
      return QObject::tr("timed out");
    case kTransferCancelled:
      return QObject::tr("cancelled");
    case kTransferStall:
      return QObject::tr("stalled (request not supported)");
    case kTransferNoDevice:
      return QObject::tr("device disconnected");
    case kTransferOverflow:
      return QObject::tr("buffer overflow");
    default:
      return QObject::tr("transfer error");
  }
}

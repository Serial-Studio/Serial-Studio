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

#include <cstddef>
#include <cstdint>
#include <QByteArrayView>
#include <QList>

#include "Protocols/Iec104/Asdu.h"

using namespace IO::Drivers;

/**
 * @brief Drives the IEC 60870-5-104 application layer: the six-octet header, the per-type element
 *        widths and both addressing modes. A misread width shifts every object after it, so the
 *        whole object list is decoded rather than only its header.
 */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  const QByteArrayView asdu(reinterpret_cast<const char*>(data), static_cast<qsizetype>(size));

  Iec104Proto::Header header;
  if (Iec104Proto::decodeHeader(asdu, header) != Iec104Proto::DecodeResult::Ok)
    return 0;

  QList<Iec104Proto::Point> points;
  (void)Iec104Proto::decode(asdu, header, points);

  for (const auto& point : points)
    (void)Iec104Proto::slotKey(point.ioa, point.typeId);

  return 0;
}

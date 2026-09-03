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

#include "IO/Drivers/Iec104/Apci.h"

using namespace IO::Drivers;

/**
 * @brief Drives the IEC 60870-5-104 transport layer: the APDU framing, its sequence numbers and
 *        the payload window a decoded frame addresses inside the caller's buffer. The connection
 *        owns no timer, so the clock is supplied here and the whole state machine stays pure.
 */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  const QByteArrayView buffer(reinterpret_cast<const char*>(data), static_cast<qsizetype>(size));

  Iec104Proto::Connection connection;
  connection.reset(0);

  qsizetype consumed = 0;
  for (int frame = 0; frame < 64 && consumed < buffer.size(); ++frame) {
    Iec104Proto::Apdu apdu;
    const auto result = connection.consume(buffer.sliced(consumed), apdu, frame);
    if (result != Iec104Proto::ParseResult::Ok || apdu.apduSize <= 0)
      break;

    consumed += apdu.apduSize;
  }

  return 0;
}

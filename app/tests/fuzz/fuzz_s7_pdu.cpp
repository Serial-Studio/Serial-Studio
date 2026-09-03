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
#include <QString>

#include "IO/Drivers/S7/S7Pdu.h"

using namespace IO::Drivers;

/**
 * @brief Drives the S7 application layer with controller-supplied bytes: the protocol header, the
 *        setup acknowledgement whose length fixes every later chunk, and the read acknowledgement
 *        walk that addresses each item INSIDE this buffer. Every length here is the controller's
 *        choice, so none of them may reach an assertion (spec 0075 E1).
 */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  const QByteArrayView pdu(reinterpret_cast<const char*>(data), static_cast<qsizetype>(size));

  S7Comm::PduHeader header;
  (void)S7Comm::parseHeader(pdu, header);

  S7Comm::PduCodec codec;
  (void)codec.parseSetupResponse(pdu);

  const int expected = size ? (1 + (data[0] % S7Comm::kMaxItemsPerRequest)) : 1;
  QList<S7Comm::ReadResult> results;
  (void)codec.parseReadResponse(pdu, expected, results);

  QString error;
  const auto address = S7Address::parse(QStringLiteral("MD0:REAL"), error);
  if (!S7Address::isValid(address))
    return 0;

  for (const auto& item : results) {
    if (item.offset < 0 || item.size < 0 || item.offset + item.size > pdu.size())
      continue;

    (void)S7Comm::decodeValue(address, pdu.sliced(item.offset, item.size));
  }

  return 0;
}

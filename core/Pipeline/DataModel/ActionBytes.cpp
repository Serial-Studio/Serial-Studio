/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "DataModel/ActionBytes.h"

#include "Core/Checksum.h"
#include "Core/SerialStudio.h"
#include "DataModel/TextCodec.h"

/**
 * @brief Encodes a TX payload to the on-wire byte array: payload (hex or escaped text through
 *        the codec), then EOL bytes, then the named checksum over everything before it.
 */
QByteArray DataModel::encode_tx(const TxPayload& spec)
{
  QByteArray b;
  const auto enc = static_cast<SerialStudio::TextEncoding>(spec.encoding);
  if (spec.hex)
    b = SerialStudio::hexToBytes(spec.payload);
  else
    b = SerialStudio::encodeText(SerialStudio::resolveEscapeSequences(spec.payload), enc);

  b.append(spec.eolBytes);

  if (!spec.checksum.isEmpty()) {
    const auto crc = IO::checksum(spec.checksum, b);
    b.append(crc);
  }

  return b;
}

/**
 * @brief Encodes an Action's TX payload through encode_tx(); the Action's EOL is an escape
 *        sequence resolved here (raw UTF-8 for binary payloads, codec-encoded for text).
 */
QByteArray DataModel::get_tx_bytes(const Action& action)
{
  TxPayload spec;
  spec.hex      = action.binaryData;
  spec.encoding = action.txEncoding;
  spec.payload  = action.txData;
  spec.checksum = action.checksum;

  if (!action.eolSequence.isEmpty()) {
    const auto enc = static_cast<SerialStudio::TextEncoding>(action.txEncoding);
    const auto eol = SerialStudio::resolveEscapeSequences(action.eolSequence);
    spec.eolBytes  = action.binaryData ? eol.toUtf8() : SerialStudio::encodeText(eol, enc);
  }

  return encode_tx(spec);
}

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

#pragma once

#include <QByteArray>
#include <QString>

#include "Core/DataModel/Frame.h"

/**
 * @file ActionBytes.h
 * @brief The on-wire encoding of a TX payload (spec 0091: one encoder for console sends and
 *        Actions). Apart from the value types because it goes through the text codecs
 *        (Qt Core5Compat), which the Core library does not link.
 */

namespace DataModel {

/**
 * @brief One TX payload ready to encode: the caller owns its EOL representation and passes
 *        resolved EOL bytes; the checksum is named (IO::availableChecksums(), "" = none) and
 *        covers the payload including the EOL.
 */
struct TxPayload {
  bool hex     = false;  ///< Payload is a hex byte string instead of text
  int encoding = 0;      ///< SerialStudio::TextEncoding for text payloads
  QString payload;       ///< User payload (text with escapes, or hex pairs)
  QString checksum;      ///< Checksum name appended last ("" = none)
  QByteArray eolBytes;   ///< Resolved end-of-line bytes appended before the checksum
};

[[nodiscard]] QByteArray encode_tx(const TxPayload& spec);
[[nodiscard]] QByteArray get_tx_bytes(const Action& action);

}  // namespace DataModel

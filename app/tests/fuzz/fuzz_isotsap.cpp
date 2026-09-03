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
#include <QByteArray>
#include <QByteArrayView>

#include "IO/Drivers/S7/IsoTsap.h"

using namespace IO::Drivers;

/**
 * @brief Drives the ISO-on-TCP transport the way a socket delivers it: one buffer that may hold a
 *        partial TPKT, a hostile length, or a connect confirmation that contradicts its own
 *        parameter list. The transport owns no buffer, so the whole framing runs with no socket.
 */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  const QByteArrayView buffer(reinterpret_cast<const char*>(data), static_cast<qsizetype>(size));

  S7Comm::Transport transport;
  (void)transport.parseConnectConfirm(buffer);

  S7Comm::Tpkt frame;
  if (transport.extractTpkt(buffer, frame) == S7Comm::TpktResult::Ok) {
    const auto tpdu = buffer.sliced(frame.tpduOffset, frame.tpduSize);

    QByteArray assembly;
    bool complete = false;
    (void)transport.acceptData(tpdu, assembly, complete);
  }

  return 0;
}

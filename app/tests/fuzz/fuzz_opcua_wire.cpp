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

#include "IO/Drivers/OpcUaWire.h"

using namespace IO::Drivers;

/**
 * @brief Walks an OPC UA delta frame the way the native template does. The frame is produced by
 *        this build, but the same decoder latches frames replayed from a recording or handed over
 *        by the API, so its entry walk is driven with arbitrary bytes.
 */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  const QByteArrayView frame(reinterpret_cast<const char*>(data), static_cast<qsizetype>(size));
  if (frame.isEmpty() || static_cast<std::uint8_t>(frame[0]) != OpcUaWire::kWireVersion)
    return 0;

  qsizetype pos = OpcUaWire::kHeaderBytes;
  OpcUaWire::Entry entry;
  for (int i = 0; i < OpcUaWire::kMaxTags && OpcUaWire::readEntry(frame, pos, entry); ++i)
    (void)entry.text.size();

  return 0;
}

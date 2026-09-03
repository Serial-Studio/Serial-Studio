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
#include <QString>

#include "IO/Drivers/MQTT/SparkplugPayload.h"

using namespace IO::Drivers;

/**
 * @brief Drives the Sparkplug B protobuf reader with broker-supplied bytes: varints that may run
 *        past their budget, length-delimited blocks that may overrun the payload, and metric
 *        datatypes this build does not represent. Every cap here bounds attacker-chosen input.
 */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  const QByteArrayView payload(reinterpret_cast<const char*>(data), static_cast<qsizetype>(size));

  QString error;
  SparkplugB::Payload decoded;
  if (!SparkplugB::decodePayload(payload, decoded, &error))
    return 0;

  for (const auto& metric : decoded.metrics)
    (void)metric.name.size();

  return 0;
}

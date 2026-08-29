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

#pragma once

#include <QJsonObject>
#include <QString>

#include "IO/Drivers/OpcUaWire.h"

namespace IO {
namespace Drivers {

/**
 * @brief One subscribed variable node and how it maps onto the wire/dataset layout. Lives in its
 *        own header so the browse, frame-assembly and picker units can speak it without pulling
 *        in the driver facade that owns them.
 */
struct OpcUaTag {
  QString nodeId;
  QString name;
  QString path;
  QString unit;
  OpcUaWire::Type type;
  int arrayLen;
  double min;
  double max;

  OpcUaTag() : type(OpcUaWire::Type::Invalid), arrayLen(1), min(0), max(0) {}

  [[nodiscard]] bool operator==(const OpcUaTag& other) const noexcept
  {
    return nodeId == other.nodeId && name == other.name && path == other.path && unit == other.unit
        && type == other.type && arrayLen == other.arrayLen
        && qFuzzyCompare(min + 1.0, other.min + 1.0) && qFuzzyCompare(max + 1.0, other.max + 1.0);
  }
};

/**
 * @brief The wire type a tag encodes as (its declared type; strings for anything unmapped).
 */
[[nodiscard]] inline OpcUaWire::Type wireTypeFor(const OpcUaTag& tag) noexcept
{
  if (tag.type == OpcUaWire::Type::Invalid)
    return OpcUaWire::Type::Str;

  return tag.type;
}

[[nodiscard]] OpcUaTag tagFromJson(const QJsonObject& obj);
[[nodiscard]] QJsonObject tagToJson(const OpcUaTag& tag);

}  // namespace Drivers
}  // namespace IO

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

#include "IO/Drivers/OpcUa/OpcUaTag.h"

#include "SerialStudio.h"

/**
 * @brief {id, name, path, unit, t, n} -> OpcUaTag.
 */
IO::Drivers::OpcUaTag IO::Drivers::tagFromJson(const QJsonObject& obj)
{
  OpcUaTag tag;
  tag.nodeId   = obj.value(QStringLiteral("id")).toString();
  tag.name     = obj.value(QStringLiteral("name")).toString(tag.nodeId);
  tag.path     = obj.value(QStringLiteral("path")).toString();
  tag.unit     = obj.value(QStringLiteral("unit")).toString();
  tag.type     = OpcUaWire::typeFromCode(obj.value(QStringLiteral("t")).toString());
  tag.arrayLen = qBound(1, obj.value(QStringLiteral("n")).toInt(1), OpcUaWire::kMaxTags);
  tag.min      = SerialStudio::toDouble(obj.value(QStringLiteral("min")));
  tag.max      = SerialStudio::toDouble(obj.value(QStringLiteral("max")));
  return tag;
}

/**
 * @brief OpcUaTag -> {id, name, path, unit, t, n}.
 */
QJsonObject IO::Drivers::tagToJson(const OpcUaTag& tag)
{
  return QJsonObject{
    {QStringLiteral("id"), tag.nodeId},
    {QStringLiteral("name"), tag.name},
    {QStringLiteral("path"), tag.path},
    {QStringLiteral("unit"), tag.unit},
    {QStringLiteral("t"), OpcUaWire::codeFromType(wireTypeFor(tag))},
    {QStringLiteral("n"), qMax(1, tag.arrayLen)},
    {QStringLiteral("min"), tag.min},
    {QStringLiteral("max"), tag.max},
  };
}

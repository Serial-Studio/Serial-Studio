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

#include "API/SchemaBuilder.h"

/**
 * @brief Lower SchemaProp into the JSON object emitted under "properties".
 */
QJsonObject API::schemaPropToJson(const SchemaProp& p)
{
  QJsonObject prop;

  if (p.type.contains(QLatin1Char('|'))) {
    QJsonArray types;
    for (const auto& part : p.type.split(QLatin1Char('|'), Qt::SkipEmptyParts))
      types.append(part.trimmed());

    prop.insert(QStringLiteral("type"), types);
  } else {
    prop.insert(QStringLiteral("type"), p.type);
  }

  prop.insert(QStringLiteral("description"), p.description);
  if (p.binary)
    prop.insert(QStringLiteral("binary"), true);

  if (!p.enumValues.isEmpty())
    prop.insert(QStringLiteral("enum"), p.enumValues);

  if (!p.minimum.isUndefined() && !p.minimum.isNull())
    prop.insert(QStringLiteral("minimum"), p.minimum);

  if (!p.maximum.isUndefined() && !p.maximum.isNull())
    prop.insert(QStringLiteral("maximum"), p.maximum);

  if (!p.defaultValue.isUndefined() && !p.defaultValue.isNull())
    prop.insert(QStringLiteral("default"), p.defaultValue);

  if (p.type == QStringLiteral("array")) {
    if (!p.itemsSchema.isEmpty()) {
      prop.insert(QStringLiteral("items"), p.itemsSchema);
    } else {
      QJsonObject items;
      items.insert(QStringLiteral("type"),
                   p.itemsType.isEmpty() ? QStringLiteral("string") : p.itemsType);
      prop.insert(QStringLiteral("items"), items);
    }
  }

  return prop;
}

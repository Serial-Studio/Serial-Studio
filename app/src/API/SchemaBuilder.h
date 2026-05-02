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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <initializer_list>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <utility>

namespace API {
/**
 * @brief One JSON-Schema property descriptor: name, JSON type, human description.
 */
struct SchemaProp {
  QString name;
  QString type;
  QString description;
};

/**
 * @brief Build the empty JSON-Schema used by parameterless commands.
 */
[[nodiscard]] inline QJsonObject emptySchema()
{
  QJsonObject schema;
  schema.insert(QStringLiteral("type"), QStringLiteral("object"));
  schema.insert(QStringLiteral("properties"), QJsonObject());
  return schema;
}

/**
 * @brief Build a JSON-Schema object from a flat list of properties.
 *
 * Every property in @p props is inserted into "properties" and listed
 * in "required" -- this matches the all-required convention used by the
 * existing handlers.
 */
[[nodiscard]] inline QJsonObject makeSchema(std::initializer_list<SchemaProp> props)
{
  QJsonObject properties;
  QJsonArray required;
  for (const auto& p : props) {
    QJsonObject prop;
    prop.insert(QStringLiteral("type"), p.type);
    prop.insert(QStringLiteral("description"), p.description);
    properties.insert(p.name, prop);
    required.append(p.name);
  }

  QJsonObject schema;
  schema.insert(QStringLiteral("type"), QStringLiteral("object"));
  schema.insert(QStringLiteral("properties"), properties);
  if (!required.isEmpty())
    schema.insert(QStringLiteral("required"), required);

  return schema;
}

/**
 * @brief Build a JSON-Schema with explicit required vs. optional properties.
 */
[[nodiscard]] inline QJsonObject makeSchema(std::initializer_list<SchemaProp> required_props,
                                            std::initializer_list<SchemaProp> optional_props)
{
  QJsonObject properties;
  QJsonArray required;
  for (const auto& p : required_props) {
    QJsonObject prop;
    prop.insert(QStringLiteral("type"), p.type);
    prop.insert(QStringLiteral("description"), p.description);
    properties.insert(p.name, prop);
    required.append(p.name);
  }

  for (const auto& p : optional_props) {
    QJsonObject prop;
    prop.insert(QStringLiteral("type"), p.type);
    prop.insert(QStringLiteral("description"), p.description);
    properties.insert(p.name, prop);
  }

  QJsonObject schema;
  schema.insert(QStringLiteral("type"), QStringLiteral("object"));
  schema.insert(QStringLiteral("properties"), properties);
  if (!required.isEmpty())
    schema.insert(QStringLiteral("required"), required);

  return schema;
}

}  // namespace API

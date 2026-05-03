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
#include <QJsonValue>
#include <QString>
#include <utility>

namespace API {
/**
 * @brief One JSON-Schema property descriptor: name, JSON type, human description.
 *
 * Optional fields (`enumValues`, `minimum`, `maximum`, `defaultValue`) are emitted
 * only when set, so existing 3-field brace-init `{name, type, description}` still
 * compiles and produces the same schema as before.
 */
struct SchemaProp {
  QString name;
  QString type;
  QString description;
  QJsonArray enumValues   = {};
  QJsonValue minimum      = {};
  QJsonValue maximum      = {};
  QJsonValue defaultValue = {};
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
 * @brief Build a SchemaProp constrained to a fixed enum, with optional default.
 */
[[nodiscard]] inline SchemaProp enumProp(QString name,
                                         QString description,
                                         QJsonArray values,
                                         QJsonValue default_value = {})
{
  SchemaProp p;
  p.name         = std::move(name);
  p.type         = QStringLiteral("integer");
  p.description  = std::move(description);
  p.enumValues   = std::move(values);
  p.defaultValue = std::move(default_value);
  return p;
}

/**
 * @brief Build a SchemaProp constrained to a numeric range.
 */
[[nodiscard]] inline SchemaProp rangeProp(QString name,
                                          QString description,
                                          QJsonValue min,
                                          QJsonValue max)
{
  SchemaProp p;
  p.name        = std::move(name);
  p.type        = QStringLiteral("integer");
  p.description = std::move(description);
  p.minimum     = std::move(min);
  p.maximum     = std::move(max);
  return p;
}

/**
 * @brief Lower SchemaProp into the JSON object emitted under "properties".
 */
[[nodiscard]] inline QJsonObject schemaPropToJson(const SchemaProp& p)
{
  QJsonObject prop;
  prop.insert(QStringLiteral("type"), p.type);
  prop.insert(QStringLiteral("description"), p.description);
  if (!p.enumValues.isEmpty())
    prop.insert(QStringLiteral("enum"), p.enumValues);

  if (!p.minimum.isUndefined() && !p.minimum.isNull())
    prop.insert(QStringLiteral("minimum"), p.minimum);

  if (!p.maximum.isUndefined() && !p.maximum.isNull())
    prop.insert(QStringLiteral("maximum"), p.maximum);

  if (!p.defaultValue.isUndefined() && !p.defaultValue.isNull())
    prop.insert(QStringLiteral("default"), p.defaultValue);

  return prop;
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
    properties.insert(p.name, schemaPropToJson(p));
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
    properties.insert(p.name, schemaPropToJson(p));
    required.append(p.name);
  }

  for (const auto& p : optional_props)
    properties.insert(p.name, schemaPropToJson(p));

  QJsonObject schema;
  schema.insert(QStringLiteral("type"), QStringLiteral("object"));
  schema.insert(QStringLiteral("properties"), properties);
  if (!required.isEmpty())
    schema.insert(QStringLiteral("required"), required);

  return schema;
}

}  // namespace API

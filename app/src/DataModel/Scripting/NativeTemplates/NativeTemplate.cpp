/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"

#include <QCoreApplication>

#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Latched parser base
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initializes the latch row with @p count zero values.
 */
DataModel::NativeLatchParser::NativeLatchParser(int count)
  : m_count(qMax(1, count)), m_values(QList<QString>(qMax(1, count), QStringLiteral("0")))
{
  Q_ASSERT(count >= 1);
  Q_ASSERT(m_values.size() == m_count);
}

/**
 * @brief Clears the latch row back to zero values.
 */
void DataModel::NativeLatchParser::reset()
{
  m_values = QList<QString>(m_count, QStringLiteral("0"));
}

/**
 * @brief Stores a value at the latch index, ignoring out-of-range indices.
 */
void DataModel::NativeLatchParser::storeAt(int index, const QString& value)
{
  Q_ASSERT(m_count == m_values.size());

  if (index < 0 || index >= m_count)
    return;

  m_values[index] = value;
}

/**
 * @brief Returns the latch row wrapped as a single-frame result.
 */
QList<QStringList> DataModel::NativeLatchParser::latchedFrame() const
{
  Q_ASSERT(!m_values.isEmpty());
  Q_ASSERT(m_count == m_values.size());

  QList<QStringList> out;
  out.append(m_values);
  return out;
}

/**
 * @brief Returns the number of latched values.
 */
int DataModel::NativeLatchParser::latchCount() const noexcept
{
  return m_count;
}

//--------------------------------------------------------------------------------------------------
// Default params
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the params object holding every schema default for the template.
 */
QJsonObject DataModel::nativeTemplateDefaults(const DataModel::INativeTemplate& tmpl)
{
  QJsonObject defaults;
  const auto specs = tmpl.params();
  for (const auto& spec : specs)
    defaults.insert(spec.key, spec.defaultValue);

  Q_ASSERT(defaults.size() == specs.size());
  return defaults;
}

//--------------------------------------------------------------------------------------------------
// Param coercion helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a parameter spec with translated label and description.
 */
DataModel::NativeParamSpec DataModel::nativeParam(const char* key,
                                                  NativeParamType type,
                                                  const char* label,
                                                  const char* description,
                                                  const QJsonValue& defaultValue)
{
  Q_ASSERT(key != nullptr);
  Q_ASSERT(label != nullptr);

  NativeParamSpec spec;
  spec.key          = QString::fromLatin1(key);
  spec.type         = type;
  spec.label        = QCoreApplication::translate("NativeTemplates", label);
  spec.description  = QCoreApplication::translate("NativeTemplates", description);
  spec.defaultValue = defaultValue;
  return spec;
}

/**
 * @brief Reads a comma-separated key list param, trimming entries and dropping empties.
 */
QStringList DataModel::nativeKeyList(const QJsonObject& params,
                                     const QString& key,
                                     const QString& fallbackCsv)
{
  Q_ASSERT(!key.isEmpty());

  const QString csv = nativeParamString(params, key, fallbackCsv);
  QStringList keys;
  const auto parts = csv.split(QLatin1Char(','));
  for (const auto& part : parts) {
    const QString trimmed = part.trimmed();
    if (!trimmed.isEmpty())
      keys.append(trimmed);
  }

  return keys;
}

/**
 * @brief Reads a structured array param, returning an empty array when absent or mistyped.
 */
QJsonArray DataModel::nativeParamArray(const QJsonObject& params, const QString& key)
{
  Q_ASSERT(!key.isEmpty());

  const auto value = params.value(key);
  if (!value.isArray())
    return QJsonArray();

  return value.toArray();
}

/**
 * @brief Reads a string param, falling back when absent or not a string.
 */
QString DataModel::nativeParamString(const QJsonObject& params,
                                     const QString& key,
                                     const QString& fallback)
{
  Q_ASSERT(!key.isEmpty());

  const auto value = params.value(key);
  if (!value.isString())
    return fallback;

  return value.toString();
}

/**
 * @brief Reads a single-character param, falling back when absent or empty.
 */
QChar DataModel::nativeParamChar(const QJsonObject& params, const QString& key, QChar fallback)
{
  Q_ASSERT(!key.isEmpty());

  const auto value = params.value(key);
  if (!value.isString())
    return fallback;

  const QString text = value.toString();
  if (text.isEmpty())
    return fallback;

  return text.at(0);
}

/**
 * @brief Reads an integer param, falling back when absent or not numeric.
 */
int DataModel::nativeParamInt(const QJsonObject& params, const QString& key, int fallback)
{
  Q_ASSERT(!key.isEmpty());

  const auto value = params.value(key);
  if (!value.isDouble())
    return fallback;

  return value.toInt(fallback);
}

/**
 * @brief Reads a floating-point param, falling back when absent or not numeric.
 */
double DataModel::nativeParamFloat(const QJsonObject& params, const QString& key, double fallback)
{
  Q_ASSERT(!key.isEmpty());

  const auto value = params.value(key);
  if (!value.isDouble())
    return fallback;

  return SerialStudio::toDouble(value, fallback);
}

/**
 * @brief Reads a boolean param, falling back when absent or not a bool.
 */
bool DataModel::nativeParamBool(const QJsonObject& params, const QString& key, bool fallback)
{
  Q_ASSERT(!key.isEmpty());

  const auto value = params.value(key);
  if (!value.isBool())
    return fallback;

  return value.toBool(fallback);
}

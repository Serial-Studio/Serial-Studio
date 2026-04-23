/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace Misc {
/**
 * @brief Secure JSON parsing and validation with configurable bounds checking.
 */
class JsonValidator {
public:
  /**
   * @brief Configurable bounds for JSON validation.
   */
  struct Limits {
    qsizetype maxFileSize;
    int maxDepth;
    int maxArraySize;

    Limits() : maxFileSize(10 * 1024 * 1024), maxDepth(128), maxArraySize(10000) {}
  };

  /**
   * @brief Result of JSON parsing and validation.
   */
  struct ValidationResult {
    bool valid = false;
    QString errorMessage;
    QJsonDocument document;
  };

  static ValidationResult parseAndValidate(const QByteArray& data);
  static ValidationResult parseAndValidate(const QByteArray& data, const Limits& limits);

  static bool validateStructure(const QJsonValue& value,
                                const Limits& limits,
                                int currentDepth = 0);

private:
  static bool validateObject(const QJsonObject& obj, const Limits& limits, int depth);
  static bool validateArray(const QJsonArray& arr, const Limits& limits, int depth);
};

inline JsonValidator::ValidationResult JsonValidator::parseAndValidate(const QByteArray& data)
{
  return parseAndValidate(data, Limits());
}

inline JsonValidator::ValidationResult JsonValidator::parseAndValidate(const QByteArray& data,
                                                                       const Limits& limits)
{
  ValidationResult result;

  if (data.size() > limits.maxFileSize) [[unlikely]] {
    result.errorMessage = QString("JSON data exceeds maximum size limit of %1 MB")
                            .arg(limits.maxFileSize / (1024 * 1024));
    return result;
  }

  if (data.isEmpty()) [[unlikely]] {
    result.errorMessage = "JSON data is empty";
    return result;
  }

  QJsonParseError parseError;
  result.document = QJsonDocument::fromJson(data, &parseError);

  if (parseError.error != QJsonParseError::NoError) [[unlikely]] {
    result.errorMessage = QString("JSON parse error at offset %1: %2")
                            .arg(parseError.offset)
                            .arg(parseError.errorString());
    return result;
  }

  if (!validateStructure(result.document.isArray() ? QJsonValue(result.document.array())
                                                   : QJsonValue(result.document.object()),
                         limits)) [[unlikely]] {
    result.errorMessage =
      QString("JSON structure validation failed: exceeds depth (%1) or array size (%2) limits")
        .arg(limits.maxDepth)
        .arg(limits.maxArraySize);
    return result;
  }

  result.valid = true;
  return result;
}

inline bool JsonValidator::validateStructure(const QJsonValue& value,
                                             const Limits& limits,
                                             int currentDepth)
{
  if (currentDepth > limits.maxDepth) [[unlikely]]
    return false;

  if (value.isObject())
    return validateObject(value.toObject(), limits, currentDepth);
  else if (value.isArray())
    return validateArray(value.toArray(), limits, currentDepth);

  return true;
}

inline bool JsonValidator::validateObject(const QJsonObject& obj, const Limits& limits, int depth)
{
  if (depth > limits.maxDepth) [[unlikely]]
    return false;

  for (auto it = obj.constBegin(); it != obj.constEnd(); ++it)
    if (!validateStructure(it.value(), limits, depth + 1)) [[unlikely]]
      return false;

  return true;
}

inline bool JsonValidator::validateArray(const QJsonArray& arr, const Limits& limits, int depth)
{
  if (depth > limits.maxDepth) [[unlikely]]
    return false;

  if (arr.size() > limits.maxArraySize) [[unlikely]]
    return false;

  for (const auto& element : arr)
    if (!validateStructure(element, limits, depth + 1)) [[unlikely]]
      return false;

  return true;
}

}  // namespace Misc

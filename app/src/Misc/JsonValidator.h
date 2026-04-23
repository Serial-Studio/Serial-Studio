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
 *
 * Provides defense against malformed or malicious JSON files that could cause
 * excessive memory use, deep recursion, or poor performance.
 *
 * **Default Security Constraints:**
 * - Maximum file size: 10 MB
 * - Maximum recursion depth: 128 levels
 * - Maximum array size: 10,000 elements
 *
 * **Thread Safety:** All methods are thread-safe (stateless static functions).
 *
 * **Performance:** O(n) where n = JSON size, with early termination on errors.
 */
class JsonValidator {
public:
  /**
   * @brief Configurable bounds for JSON validation.
   *
   * Defines maximum allowed values for JSON structure dimensions.
   */
  struct Limits {
    qsizetype maxFileSize;
    int maxDepth;
    int maxArraySize;

    /**
     * @brief Constructs Limits with secure default values.
     */
    Limits() : maxFileSize(10 * 1024 * 1024), maxDepth(128), maxArraySize(10000) {}
  };

  /**
   * @brief Result of JSON parsing and validation.
   *
   * Contains validation status, error message (if failed), and
   * the parsed document (if successful).
   */
  struct ValidationResult {
    bool valid = false;
    QString errorMessage;
    QJsonDocument document;
  };

  /**
   * @brief Parses and validates JSON with default security limits.
   *
   * Convenience overload using default Limits. Suitable for most use cases.
   *
   * @param data Raw JSON byte array to validate
   * @return ValidationResult with status and document or error message
   */
  static ValidationResult parseAndValidate(const QByteArray& data);

  /**
   * @brief Parses and validates JSON with custom security limits.
   *
   * @param data Raw JSON byte array to validate
   * @param limits Custom validation thresholds
   * @return ValidationResult with status and document or error message
   */
  static ValidationResult parseAndValidate(const QByteArray& data, const Limits& limits);

  /**
   * @brief Recursively validates JSON structure depth and array sizes.
   *
   * @param value JSON value to validate
   * @param limits Validation thresholds
   * @param currentDepth Current recursion depth
   * @return True if structure is valid, false otherwise
   */
  static bool validateStructure(const QJsonValue& value,
                                const Limits& limits,
                                int currentDepth = 0);

private:
  /**
   * @brief Validates a JSON object and its children recursively.
   */
  static bool validateObject(const QJsonObject& obj, const Limits& limits, int depth);

  /**
   * @brief Validates a JSON array and its elements recursively.
   */
  static bool validateArray(const QJsonArray& arr, const Limits& limits, int depth);
};

/**
 * @brief Default validation using standard security limits.
 *
 * @param data JSON byte array to validate
 * @return ValidationResult with parsed document or error details
 */
inline JsonValidator::ValidationResult JsonValidator::parseAndValidate(const QByteArray& data)
{
  return parseAndValidate(data, Limits());
}

/**
 * @brief Full JSON parsing and validation implementation.
 *
 * **Validation Sequence:**
 * 1. Check file size against limits.maxFileSize
 * 2. Check for empty data
 * 3. Parse JSON syntax with QJsonDocument
 * 4. Recursively validate structure (depth and array sizes)
 *
 * @param data JSON byte array to validate
 * @param limits Validation thresholds to enforce
 * @return ValidationResult with status, document, or error message
 */
inline JsonValidator::ValidationResult JsonValidator::parseAndValidate(const QByteArray& data,
                                                                       const Limits& limits)
{
  ValidationResult result;

  // Validation Step 1: Check file size limit
  if (data.size() > limits.maxFileSize) [[unlikely]] {
    result.errorMessage = QString("JSON data exceeds maximum size limit of %1 MB")
                            .arg(limits.maxFileSize / (1024 * 1024));
    return result;
  }

  // Validation Step 2: Check for empty input
  if (data.isEmpty()) [[unlikely]] {
    result.errorMessage = "JSON data is empty";
    return result;
  }

  // Validation Step 3: Parse JSON syntax
  QJsonParseError parseError;
  result.document = QJsonDocument::fromJson(data, &parseError);

  if (parseError.error != QJsonParseError::NoError) [[unlikely]] {
    result.errorMessage = QString("JSON parse error at offset %1: %2")
                            .arg(parseError.offset)
                            .arg(parseError.errorString());
    return result;
  }

  // Validation Step 4: Validate structure depth and array sizes
  if (!validateStructure(result.document.isArray() ? QJsonValue(result.document.array())
                                                   : QJsonValue(result.document.object()),
                         limits)) [[unlikely]] {
    result.errorMessage =
      QString("JSON structure validation failed: exceeds depth (%1) or array size (%2) limits")
        .arg(limits.maxDepth)
        .arg(limits.maxArraySize);
    return result;
  }

  // All validations passed
  result.valid = true;
  return result;
}

/**
 * @brief Recursively validates JSON value depth and structure.
 *
 * Dispatches to validateObject() or validateArray() based on value type.
 * Primitive types are always valid.
 *
 * @param value JSON value to validate
 * @param limits Validation thresholds
 * @param currentDepth Current nesting level
 * @return True if value and all children are within limits
 */
inline bool JsonValidator::validateStructure(const QJsonValue& value,
                                             const Limits& limits,
                                             int currentDepth)
{
  // Check recursion depth limit
  if (currentDepth > limits.maxDepth) [[unlikely]]
    return false;

  // Dispatch based on type
  if (value.isObject())
    return validateObject(value.toObject(), limits, currentDepth);
  else if (value.isArray())
    return validateArray(value.toArray(), limits, currentDepth);

  // Primitives are always valid
  return true;
}

/**
 * @brief Validates a JSON object and all nested children.
 *
 * @param obj JSON object to validate
 * @param limits Validation thresholds
 * @param depth Current nesting depth
 * @return True if object and all children are within limits
 */
inline bool JsonValidator::validateObject(const QJsonObject& obj, const Limits& limits, int depth)
{
  // Check recursion depth limit
  if (depth > limits.maxDepth) [[unlikely]]
    return false;

  // Recursively validate each property value
  for (auto it = obj.constBegin(); it != obj.constEnd(); ++it)
    if (!validateStructure(it.value(), limits, depth + 1)) [[unlikely]]
      return false;

  return true;
}

/**
 * @brief Validates a JSON array size and all nested elements.
 *
 * @param arr JSON array to validate
 * @param limits Validation thresholds
 * @param depth Current nesting depth
 * @return True if array and all elements are within limits
 */
inline bool JsonValidator::validateArray(const QJsonArray& arr, const Limits& limits, int depth)
{
  // Check recursion depth limit
  if (depth > limits.maxDepth) [[unlikely]]
    return false;

  // Check array size limit
  if (arr.size() > limits.maxArraySize) [[unlikely]]
    return false;

  // Recursively validate each element
  for (const auto& element : arr)
    if (!validateStructure(element, limits, depth + 1)) [[unlikely]]
      return false;

  return true;
}

}  // namespace Misc

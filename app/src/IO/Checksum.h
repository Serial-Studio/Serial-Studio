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

#include <concepts>
#include <QMap>
#include <QStringList>

#include "Concepts.h"

namespace IO {
using ChecksumFunc = std::function<QByteArray(const char*, int)>;

[[nodiscard]] const QStringList& availableChecksums();
[[nodiscard]] const QMap<QString, ChecksumFunc>& checksumFunctionMap();
[[nodiscard]] QByteArray checksum(const QString& name, const QByteArray& data);

//--------------------------------------------------------------------------------------------------
// Generic checksum validation utilities using C++20 concepts
//--------------------------------------------------------------------------------------------------

/**
 * @brief Validates checksum for numeric data arrays.
 *
 * Computes checksum over a raw numeric array and compares with expected value.
 * Provides compile-time guarantee that T is a numeric type.
 *
 * **Performance:** O(n × algorithm_complexity)
 * **Thread Safety:** Safe (pure function)
 *
 * @tparam T Numeric type (int, uint8_t, double, etc.)
 * @param algorithm Checksum algorithm name (e.g., "CRC-16")
 * @param data Pointer to numeric data array
 * @param length Number of elements in array
 * @param expected Expected checksum value
 * @return true if computed checksum matches expected value
 */
template<Concepts::Numeric T>
[[nodiscard]] inline bool validateChecksum(const QString& algorithm,
                                           const T* data,
                                           int length,
                                           const QByteArray& expected) noexcept
{
  if (!data || length <= 0)
    return false;

  const auto computed =
    checksum(algorithm, QByteArray(reinterpret_cast<const char*>(data), length * sizeof(T)));
  return computed == expected;
}

/**
 * @brief Computes checksum for numeric containers.
 *
 * Generic checksum computation for any container of numeric types.
 * Provides compile-time guarantee that container holds numeric values.
 *
 * **Performance:** O(n × algorithm_complexity)
 * **Thread Safety:** Safe (pure function)
 *
 * @tparam Container Type with .data() and .size() (e.g., std::vector, QVector)
 * @param algorithm Checksum algorithm name
 * @param container Numeric data container
 * @return Computed checksum as byte array
 */
template<typename Container>
  requires requires(Container c) {
    { c.data() } -> std::convertible_to<const void*>;
    { c.size() } -> std::convertible_to<std::size_t>;
    requires Concepts::Numeric<typename Container::value_type>;
  }

[[nodiscard]] inline QByteArray computeChecksum(const QString& algorithm,
                                                const Container& container) noexcept
{
  using ValueType = typename Container::value_type;
  return checksum(algorithm,
                  QByteArray(reinterpret_cast<const char*>(container.data()),
                             container.size() * sizeof(ValueType)));
}

}  // namespace IO

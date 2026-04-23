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
#include <QIODevice>
#include <QString>

namespace Concepts {
/**
 * @concept Numeric
 * @brief Constrains template parameter to numeric types (integral or floating
 * point).
 *
 * **Satisfied by:**
 * - Integral types
 * - Floating point types
 *
 * **Use Cases:**
 * - Generic mathematical algorithms
 * - Statistical calculations
 * - Numeric data processing
 *
 * @tparam T Type to check for numeric properties
 */
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

/**
 * @concept ByteLike
 * @brief Constrains template parameter to byte-sized integral types.
 *
 * **Use Cases:**
 * - Low-level data parsing
 * - Binary protocol implementations
 * - Checksum algorithms operating on bytes
 *
 * @tparam T Type to check for byte-like properties
 */
template<typename T>
concept ByteLike = std::integral<T> && sizeof(T) == 1;

/**
 * @concept Comparable
 * @brief Constrains template parameter to types supporting equality comparison.
 *
 * @tparam T Type to check for comparison support
 */
template<typename T>
concept Comparable = requires(T a, T b) {
  { a == b } -> std::convertible_to<bool>;
  { a != b } -> std::convertible_to<bool>;
};

/**
 * @concept Hashable
 * @brief Constrains template parameter to types that can be hashed.
 *
 * @tparam T Type to check for hash support
 */
template<typename T>
concept Hashable = requires(T a) {
  { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

/**
 * @concept Serializable
 * @brief Constrains template parameter to types with read() and serialize()
 * methods.
 *
 * Ensures type T follows the Serial Studio serialization protocol used by
 * Frame, Group, Dataset, and related structures.
 *
 * @tparam T Type to check for serialization support
 */
template<typename T>
concept Serializable = requires(T obj) {
  { serialize(obj) };
  { read(obj, std::declval<const QJsonObject&>()) } -> std::same_as<bool>;
};

/**
 * @concept Frameable
 * @brief Constrains template parameter to types that represent data frames.
 *
 * Ensures type T has groups, actions, and title members, matching the Frame
 * structure.
 *
 * @tparam T Type to check for frame properties
 */
template<typename T>
concept Frameable = requires(T frame) {
  { frame.groups };
  { frame.actions };
  { frame.title } -> std::convertible_to<QString>;
};

/**
 * @concept Driver
 * @brief Constrains template parameter to types implementing HAL_Driver
 * interface.
 *
 * Ensures type T provides the core I/O driver methods: open(), close(),
 * write(), and state queries.
 *
 * @tparam T Type to check for driver interface compliance
 */
template<typename T>
concept Driver = requires(T driver) {
  { driver.open(std::declval<QIODevice::OpenMode>()) } -> std::same_as<bool>;
  { driver.close() } -> std::same_as<void>;
  { driver.isOpen() } -> std::same_as<bool>;
  { driver.isReadable() } -> std::same_as<bool>;
  { driver.isWritable() } -> std::same_as<bool>;
  { driver.write(std::declval<const QByteArray&>()) } -> std::convertible_to<quint64>;
};

}  // namespace Concepts

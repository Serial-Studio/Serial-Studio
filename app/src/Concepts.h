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
 * @brief Constrains a type to numeric (integral or floating-point).
 */
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

/**
 * @brief Constrains a type to byte-sized integrals.
 */
template<typename T>
concept ByteLike = std::integral<T> && sizeof(T) == 1;

/**
 * @brief Constrains a type to support equality comparison.
 */
template<typename T>
concept Comparable = requires(T a, T b) {
  { a == b } -> std::convertible_to<bool>;
  { a != b } -> std::convertible_to<bool>;
};

/**
 * @brief Constrains a type to be hashable via std::hash.
 */
template<typename T>
concept Hashable = requires(T a) {
  { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

/**
 * @brief Constrains a type to Serial Studio's read()/serialize() protocol.
 */
template<typename T>
concept Serializable = requires(T obj) {
  { serialize(obj) };
  { read(obj, std::declval<const QJsonObject&>()) } -> std::same_as<bool>;
};

/**
 * @brief Constrains a type to look like a data frame (groups, actions, title).
 */
template<typename T>
concept Frameable = requires(T frame) {
  { frame.groups };
  { frame.actions };
  { frame.title } -> std::convertible_to<QString>;
};

/**
 * @brief Constrains a type to implement the HAL_Driver I/O interface.
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

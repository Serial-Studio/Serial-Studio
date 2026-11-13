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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
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

}  // namespace Concepts
